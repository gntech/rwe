#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/geometry/CollisionMesh.h>
#include "MeshService.h"
#include "_3do.h"
#include "Gaf.h"
#include "BoxTreeSplit.h"

namespace rwe
{
    float convertFixedPoint(int p)
    {
        return static_cast<float>(p) / 65536.0f;
    }

    Vector3f vertexToVector(const _3do::Vertex& v)
    {
        return Vector3f(
            convertFixedPoint(-v.x), // flip the x axis
            convertFixedPoint(v.y),
            convertFixedPoint(v.z));
    }

    struct FrameInfo
    {
        std::string name;
        unsigned int frameNumber;
        Grid<char> data;

        FrameInfo(const std::string& name, unsigned int frameNumber, unsigned int width, unsigned int height)
            : name(name), frameNumber(frameNumber), data(width, height)
        {
        }
    };

    class FrameListGafAdapter : public GafReaderAdapter
    {
    private:
        std::vector<FrameInfo>* frames;
        const std::string* entryName;
        FrameInfo* frameInfo;
        GafFrameData currentFrameHeader;
        unsigned int frameNumber{0};

    public:
        explicit FrameListGafAdapter(std::vector<FrameInfo>* frames, const std::string* entryName)
            : frames(frames),
              entryName(entryName)
        {
        }

        void beginFrame(const GafFrameData& header) override
        {
            frameInfo = &(frames->emplace_back(*entryName, frameNumber, header.width, header.height));
            currentFrameHeader = header;
        }

        void frameLayer(const LayerData& data) override
        {
            for (std::size_t y = 0; y < data.height; ++y)
            {
                for (std::size_t x = 0; x < data.width; ++x)
                {
                    auto outPosX = static_cast<int>(x) - (data.x - currentFrameHeader.posX);
                    auto outPosY = static_cast<int>(y) - (data.y - currentFrameHeader.posY);

                    if (outPosX < 0 || outPosX >= currentFrameHeader.width || outPosY < 0 || outPosY >= currentFrameHeader.height)
                    {
                        throw std::runtime_error("frame coordinate out of bounds");
                    }

                    auto colorIndex = static_cast<unsigned char>(data.data[(y * data.width) + x]);
                    if (colorIndex == data.transparencyKey)
                    {
                        continue;
                    }

                    frameInfo->data.set(outPosX, outPosY, colorIndex);
                }
            }
        }

        void endFrame() override
        {
            ++frameNumber;
        }
    };

    MeshService MeshService::createMeshService(AbstractVirtualFileSystem* vfs, GraphicsContext* graphics, const ColorPalette* palette)
    {
        auto gafs = vfs->getFileNames("textures", ".gaf");

        std::vector<FrameInfo> frames;

        // load all the textures into memory
        for (const auto& gafName : gafs)
        {
            auto bytes = vfs->readFile("textures/" + gafName);
            if (!bytes)
            {
                throw std::runtime_error("File in listing could not be read: " + gafName);
            }

            auto stream = boost::interprocess::bufferstream(bytes->data(), bytes->size());
            GafArchive gaf(&stream);

            for (const auto& e : gaf.entries())
            {
                FrameListGafAdapter adapter(&frames, &e.name);
                gaf.extract(e, adapter);
            }
        }

        // figure out how to pack the textures into an atlas
        std::vector<FrameInfo*> frameRefs;
        frameRefs.reserve(frames.size());
        for (auto& f : frames)
        {
            frameRefs.push_back(&f);
        }

        auto packInfo = packGridsGeneric<FrameInfo*>(frameRefs, [](const FrameInfo* f) {
            return Size(f->data.getWidth(), f->data.getHeight());
        });

        // pack the textures
        Grid<Color> atlas(packInfo.width, packInfo.height);
        std::unordered_map<FrameId, Rectangle2f> atlasMap;

        for (const auto& e : packInfo.entries)
        {
            FrameId id(e.value->name, e.value->frameNumber);

            auto left = static_cast<float>(e.x) / static_cast<float>(packInfo.width);
            auto top = static_cast<float>(e.y) / static_cast<float>(packInfo.height);
            auto right = static_cast<float>(e.x + e.value->data.getWidth()) / static_cast<float>(packInfo.width);
            auto bottom = static_cast<float>(e.y + e.value->data.getHeight()) / static_cast<float>(packInfo.height);
            auto bounds = Rectangle2f::fromTLBR(top, left, bottom, right);

            atlasMap.insert({id, bounds});

            atlas.transformAndReplaceArea<char>(e.x, e.y, e.value->data, [palette](char v) {
                return (*palette)[static_cast<unsigned char>(v)];
            });
        }

        SharedTextureHandle atlasTexture(graphics->createTexture(atlas));

        return MeshService(vfs, palette, std::move(atlasTexture), std::move(atlasMap));
    }

    MeshService::MeshService(
        AbstractVirtualFileSystem* vfs,
        const ColorPalette* palette,
        SharedTextureHandle&& atlas,
        std::unordered_map<FrameId, Rectangle2f>&& atlasMap)
        : vfs(vfs),
          palette(palette),
          atlas(std::move(atlas)),
          atlasMap(std::move(atlasMap))
    {
    }

    Mesh MeshService::meshFrom3do(const _3do::Object& o)
    {
        Mesh m;
        m.texture = getMeshTextureAtlas();

        for (const auto& p : o.primitives)
        {
            // handle textured quads
            if (p.vertices.size() == 4 && p.textureName)
            {
                auto textureBounds = getTextureRegion(*(p.textureName), 0);

                Mesh::Triangle t0(
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[2]]), textureBounds.bottomRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[1]]), textureBounds.topRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[0]]), textureBounds.topLeft()));

                Mesh::Triangle t1(
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[3]]), textureBounds.bottomLeft()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[2]]), textureBounds.bottomRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[0]]), textureBounds.topLeft()));

                m.faces.push_back(t0);
                m.faces.push_back(t1);

                continue;
            }

            // handle other polygon types
            if (p.vertices.size() >= 3)
            {
                const auto& first = vertexToVector(o.vertices[p.vertices.front()]);
                for (unsigned int i = p.vertices.size() - 1; i >= 2; --i)
                {
                    const auto& second = vertexToVector(o.vertices[p.vertices[i]]);
                    const auto& third = vertexToVector(o.vertices[p.vertices[i - 1]]);
                    Mesh::Triangle t(
                        Mesh::Vertex(first, Vector2f(0.0f, 0.0f)),
                        Mesh::Vertex(second, Vector2f(0.0f, 0.0f)),
                        Mesh::Vertex(third, Vector2f(0.0f, 0.0f)),
                        (*palette)[p.colorIndex]);
                    m.colorFaces.push_back(t);
                }

                continue;
            }

            // just ignore degenerate faces (0, 1 or 2 vertices)
        }

        return m;
    }

    UnitMesh MeshService::unitMeshFrom3do(const _3do::Object& o)
    {
        UnitMesh m;
        m.origin = Vector3f(
            convertFixedPoint(-o.x), // flip the x axis
            convertFixedPoint(o.y),
            convertFixedPoint(o.z));
        m.name = o.name;
        m.mesh = std::make_shared<ShaderMesh>(graphics->convertMesh(meshFrom3do(o)));

        for (const auto& c : o.children)
        {
            m.children.push_back(unitMeshFrom3do(c));
        }

        return m;
    }

    MeshService::UnitMeshInfo MeshService::loadUnitMesh(const std::string& name)
    {
        auto bytes = vfs->readFile("objects3d/" + name + ".3do");
        if (!bytes)
        {
            throw std::runtime_error("Failed to load object bytes: " + name);
        }

        boost::interprocess::bufferstream s(bytes->data(), bytes->size());
        auto objects = parse3doObjects(s, s.tellg());
        assert(objects.size() == 1);
        auto selectionMesh = selectionMeshFrom3do(objects.front());
        auto unitMesh = unitMeshFrom3do(objects.front());
        return UnitMeshInfo{unitMesh, selectionMesh};
    }

    SharedTextureHandle MeshService::getMeshTextureAtlas()
    {
        return atlas;
    }

    Rectangle2f MeshService::getTextureRegion(const std::string& name, unsigned int frameNumber)
    {
        FrameId frameId(name, frameNumber);
        auto it = atlasMap.find(frameId);
        if (it == atlasMap.end())
        {
            throw std::runtime_error("Texture not found in atlas: " + name);
        }

        return it->second;
    }

    rwe::CollisionMesh MeshService::selectionMeshFrom3do(const _3do::Object& o)
    {
        assert(!!o.selectionPrimitiveIndex);
        auto index = *o.selectionPrimitiveIndex;
        auto p = o.primitives.at(index);

        assert(p.vertices.size() == 4);
        Vector3f offset(convertFixedPoint(o.x), convertFixedPoint(o.y), convertFixedPoint(o.z));

        auto a = offset + vertexToVector(o.vertices[p.vertices[0]]);
        auto b = offset + vertexToVector(o.vertices[p.vertices[1]]);
        auto c = offset + vertexToVector(o.vertices[p.vertices[2]]);
        auto d = offset + vertexToVector(o.vertices[p.vertices[3]]);

        return CollisionMesh::fromQuad(a, b, c, d);
    }
}
