#ifndef RWE_ASTARPATHFINDER_H
#define RWE_ASTARPATHFINDER_H

#include <boost/optional.hpp>
#include <deque>
#include <rwe/MinHeap.h>
#include <unordered_map>
#include <vector>

namespace rwe
{
    /**
     * The maximum number of elements in the open list to expand
     * before giving up on a path search.
     */
    const unsigned int MaxOpenListQueries = 100;

    template <typename T, typename Cost = float>
    struct AStarVertexInfo
    {
        Cost costToReach;
        T vertex;
        boost::optional<const AStarVertexInfo<T, Cost>*> predecessor;
    };

    enum class AStarPathType
    {
        Complete,
        Partial
    };

    template <typename T>
    struct AStarPathInfo
    {
        AStarPathType type;
        std::vector<T> path;
    };

    template <typename T, typename Cost = float>
    class AStarPathFinder
    {
    public:
        using VertexInfo = AStarVertexInfo<T, Cost>;

    public:
        AStarPathInfo<T> findPath(const T& start)
        {
            auto openVertices = createMinHeap<T, std::pair<Cost, VertexInfo>>(
                [](const auto& p) { return p.second.vertex; },
                [](const auto& a, const auto& b) { return a.first < b.first; });
            openVertices.pushOrDecrease({estimateCostToGoal(start), VertexInfo{Cost(), start, boost::none}});

            std::unordered_map<T, VertexInfo> closedVertices;

            boost::optional<std::pair<Cost, const VertexInfo*>> closestVertex;

            unsigned int openListQueriesPerformed = 0;

            while (!openVertices.empty() && openListQueriesPerformed++ < MaxOpenListQueries)
            {
                const std::pair<Cost, VertexInfo>& openFront = openVertices.top();
                const VertexInfo& current = closedVertices[openFront.second.vertex] = openFront.second;
                openVertices.pop();

                if (isGoal(current.vertex))
                {
                    return AStarPathInfo<T>{AStarPathType::Complete, walkPath(current)};
                }

                if (!closestVertex || openFront.first < closestVertex->first)
                {
                    closestVertex = std::pair<Cost, const VertexInfo*>(openFront.first, &current);
                }

                for (const VertexInfo& s : getSuccessors(current))
                {
                    if (closedVertices.find(s.vertex) != closedVertices.end())
                    {
                        continue;
                    }

                    auto estimatedTotalCost = s.costToReach + estimateCostToGoal(s.vertex);
                    openVertices.pushOrDecrease({estimatedTotalCost, s});
                }
            }

            return AStarPathInfo<T>{AStarPathType::Partial, walkPath(*(closestVertex->second))};
        }

    protected:
        virtual bool isGoal(const T& vertex) = 0;

        virtual Cost estimateCostToGoal(const T& vertex) = 0;

        virtual std::vector<VertexInfo> getSuccessors(const VertexInfo& vertex) = 0;

    private:
        std::vector<T> walkPath(const VertexInfo& info)
        {
            std::vector<T> items;
            boost::optional<const VertexInfo*> v = &info;
            while (v)
            {
                items.push_back((*v)->vertex);
                v = (*v)->predecessor;
            }

            std::reverse(items.begin(), items.end());
            return items;
        }
    };
}

#endif
