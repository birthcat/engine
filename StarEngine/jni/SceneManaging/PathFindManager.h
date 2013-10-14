#pragma once
#include <vector>
#include "SearchCell.h"
#include "../defines.h"

namespace star
{
	struct Context;
	class Object;

	enum Direction
	{
		GoingUp,
		GoingLeft,
		GoingDown,
		GoingRight,
		GoingUpLeft,
		GoingUpRight,
		GoingDownLeft,
		GoingDownRight
	};

	class PathFindManager
	{
	public:
		~PathFindManager(void);
		static PathFindManager* GetInstance();

		void AddObject(Object* object);
		void RemoveObject(const Object* object);
		
		void AddObstacle(Object* object);

#ifdef STAR2D
		void FindPath(const vec2& currentPos, const vec2& targetPos);
		vec2 NextPathPos(Object* enemy);
		vec2 GetStep(uint16 step);
#else
		void FindPath(const vec3& currentPos, const vec3& targetPos);
		vec3 NextPathPos(Object* enemy);
		vec3 GetStep(uint16 step);
#endif

		void ClearOpenList() { m_OpenList.clear(); }
		void ClearVisitedList() { m_VisitedList.clear(); }
		void ClearPathToGoal() { m_PathToGoal.clear(); }
		bool GetInitializedGoal() const { return m_bInitializedStartGoal; }
		bool GetFoundGoal() const { m_bFoundGoal; }
	private:
		PathFindManager(void);
		void SetStartAndGoal(const SearchCell& start, const SearchCell& end);
		void PathOpened(int x, int y, float newCost, SearchCell *parent, Direction dir);
		SearchCell *GetNextCell();
		void ContinuePath();

		static PathFindManager* m_pPathFindManager;
		
		//Contains the object* and positions where you can walk on
		std::vector<Object*> m_ObjectList;

#ifdef STAR2D
		std::vector<vec2> m_PositionList;
#else
		std::vector<vec3> m_PositionList;
#endif

		//Containers for a* algorithm
		SearchCell *m_pStartCell, *m_pEndCell;
		std::vector<SearchCell*> m_OpenList;
		std::vector<SearchCell*> m_VisitedList;

#ifdef STAR2D
		std::vector<vec2> m_PathToGoal;
#else
		std::vector<vec3> m_PathToGoal;
#endif

		bool m_bInitializedStartGoal;
		bool m_bFoundGoal;

		static const int WORLD_SIZE = 100;
		static const int STEP_SIZE = 1;

#ifdef STAR2D
		static const vec2 NO_PATH_AVAILBLE;
#else
		static const vec3 NO_PATH_AVAILBLE;
#endif

		PathFindManager(const PathFindManager& yRef);
		PathFindManager(PathFindManager&& yRef);
		PathFindManager& operator=(const PathFindManager& yRef);
	};
}

