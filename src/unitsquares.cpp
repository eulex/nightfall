/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "unitsquares.h"

#include "unit.h"
#include "aipathfinding.h"
#include "environment.h"
#include "unittype-pre.h"
#include <set>
#include <iostream>

using namespace std;

namespace Game
{
	namespace Dimension
	{
		std::vector<gc_ptr<Unit> >**** unitsInBigSquaresPerPlayer;
		std::vector<gc_ptr<Unit> >*** unitsInBigSquares;
		char****      movementTypeWithSizeCanWalkOnSquare;
		char***       traversalTimeBySize;
		int bigSquareHeight, bigSquareWidth;
		int bigSquareRightShift = 4;
		gc_root_ptr<RangeArray>::type nextToRangeArray;
		GLfloat unitBuildingMaximumAltitude = -0.3f;

		void GetWHUpperLeftCorner(int width, int height, float mx, float my, int& lx, int& uy)
		{
			lx = (int) mx - (width>>1);
			uy = (int) my - (height>>1);
		}

		void GetWHUpperLeftCorner(int width, int height, int mx, int my, int& lx, int& uy)
		{
			lx = mx - (width>>1);
			uy = my - (width>>1);
		}

		void GetSizeUpperLeftCorner(int size, float mx, float my, int& lx, int& uy)
		{
			lx = (int) mx - (size>>1);
			uy = (int) my - (size>>1);
		}

		void GetSizeUpperLeftCorner(int size, int mx, int my, int& lx, int& uy)
		{
			lx = mx - (size>>1);
			uy = my - (size>>1);
		}

		void GetTypeUpperLeftCorner(const gc_ptr<UnitType>& type, int mx, int my, int& lx, int& uy)
		{
			lx = mx - (type->widthOnMap>>1);
			uy = my - (type->heightOnMap>>1);
		}

		void GetTypeUpperLeftCorner(const gc_ptr<UnitType>& type, float mx, float my, int& lx, int& uy)
		{
			lx = (int) mx - (type->widthOnMap>>1);
			uy = (int) my - (type->heightOnMap>>1);
		}

		void GetTypeUpperLeftCorner(const gc_ptr<UnitType>& type, float mx, float my, float& lx, float& uy)
		{
			lx = floor(mx) - (float) (type->widthOnMap>>1);
			uy = floor(my) - (float) (type->heightOnMap>>1);
		}

		void GetUnitUpperLeftCorner(const gc_ptr<Unit>& unit, int& lx, int& uy)
		{
			lx = unit->curAssociatedSquare.x - (unit->type->widthOnMap>>1);
			uy = unit->curAssociatedSquare.y - (unit->type->heightOnMap>>1);
		}

		void GetUnitUpperLeftCorner(const gc_ptr<Unit>& unit, float mx, float my, int& lx, int& uy)
		{
			lx = (int) mx - (unit->type->widthOnMap>>1);
			uy = (int) my - (unit->type->heightOnMap>>1);
		}

		void GetUnitUpperLeftCorner(const gc_ptr<Unit>& unit, int mx, int my, int& lx, int& uy)
		{
			lx = mx - (unit->type->widthOnMap>>1);
			uy = my - (unit->type->heightOnMap>>1);
		}

		bool WithinRange(const gc_ptr<UnitType>& type, int attacker_x, int attacker_y, int target_x, int target_y, float maxrange, float minrange)
		{
			int start_x, start_y;
			int end_x, end_y;
			float distance;
			int x, y;
			GetTypeUpperLeftCorner(type, target_x, target_y, start_x, start_y);
			end_x = start_x + type->widthOnMap - 1;
			end_y = start_y + type->heightOnMap - 1;

			if (attacker_x < start_x)
			{
				x = start_x;
			}
			else if (attacker_x > end_x)
			{
				x = end_x;
			}
			else
			{
				x = attacker_x;
			}

			if (attacker_y < start_y)
			{
				y = start_y;
			}
			else if (attacker_y > end_y)
			{
				y = end_y;
			}
			else
			{
				y = attacker_y;
			}

			int dist_x = attacker_x - x, dist_y = attacker_y - y;
			int dist_both = abs(dist_x) + abs(dist_y);
			if (dist_both <= maxrange)
			{
				if (dist_both < minrange)
				{
					return false;
				}
				else
				{
					distance = (float) sqrt(float(dist_x * dist_x + dist_y * dist_y));
					if (distance >= minrange)
					{
						return true;
					}
				}
			}
			else if (fabs((float)dist_x) <= maxrange && fabs((float)dist_y) <= maxrange && dist_both + (dist_both >> 1) <= maxrange)
			{
				distance = (float) sqrt(float(dist_x * dist_x + dist_y * dist_y));
				if ((distance <= maxrange) && (distance >= minrange))
				{
					return true;
				}
			}

			return false;
		}

		bool WithinRangeArray(const gc_ptr<UnitType>& type, int attacker_x, int attacker_y, int target_x, int target_y, const gc_ptr<RangeArray>& rangeArray)
		{
			int start_x, start_y;
			int end_x, end_y;
			int x, y;
			GetTypeUpperLeftCorner(type, target_x, target_y, start_x, start_y);
			end_x = start_x + type->widthOnMap - 1;
			end_y = start_y + type->heightOnMap - 1;

			if (attacker_x < start_x)
			{
				x = start_x;
			}
			else if (attacker_x > end_x)
			{
				x = end_x;
			}
			else
			{
				x = attacker_x;
			}

			if (attacker_y < start_y)
			{
				y = start_y;
			}
			else if (attacker_y > end_y)
			{
				y = end_y;
			}
			else
			{
				y = attacker_y;
			}

			int size = rangeArray->size, offset = rangeArray->offset;
			int array_x = attacker_x - x + offset, array_y = attacker_y - y + offset;
			if (array_x >= 0 && array_x < size && array_y >= 0 && array_y < size)
			{
				return rangeArray->array[array_y][array_x];
			}

			return false;
		}

		bool WithinRange(const gc_ptr<Unit>& unit, int pos_x, int pos_y, float maxrange, float minrange)
		{
			return WithinRange(unit->type, pos_x, pos_y, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, maxrange, minrange);
		}

		bool WithinRangeArray(const gc_ptr<Unit>& unit, int pos_x, int pos_y, const gc_ptr<RangeArray> rangeArray)
		{
			return WithinRangeArray(unit->type, pos_x, pos_y, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, rangeArray);
		}

		bool WouldBeAbleToReach(const gc_ptr<Unit>& attacker, int attacker_x, int attacker_y, const gc_ptr<Unit>& target)
		{
			if(!target)
			{
				return false;
			}
			return WithinRangeArray(target, attacker_x, attacker_y, attacker->type->attackRangeArray);
		}

		float Distance2D(float x, float y)
		{
			return sqrt(pow(x, 2) + pow(y, 2));
		}

		float Distance2D(int x, int y)
		{
			return sqrt(float(x * x + y * y));
		}

		void GetNearestUnoccupiedPosition(const gc_ptr<UnitType>& type, int& x, int& y)
		{
			int num_steps = 1;
			if (SquaresAreWalkable(type, x, y, SIW_ALLKNOWING))
			{
				return;
			}

			for (int j = 0; j < 10000; j++)
			{
				for (int i = 0; i < num_steps; i++)
				{
					x++;
					if (SquaresAreWalkable(type, x, y, SIW_ALLKNOWING))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y++;
					if (SquaresAreWalkable(type, x, y, SIW_ALLKNOWING))
					{
						return;
					}
				}

				num_steps++;
				
				for (int i = 0; i < num_steps; i++)
				{
					x--;
					if (SquaresAreWalkable(type, x, y, SIW_ALLKNOWING))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y--;
					if (SquaresAreWalkable(type, x, y, SIW_ALLKNOWING))
					{
						return;
					}
				}

				num_steps++;
			}
		}

		bool IsWithinRangeForBuilding(const gc_ptr<Unit>& unit)
		{
			if (unit->type->isMobile)
			{
				if (unit->pMovementData->action.args.unitType)
				{
					return WithinRangeArray(unit->pMovementData->action.args.unitType, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, unit->pMovementData->action.goal.pos.x, unit->pMovementData->action.goal.pos.y, nextToRangeArray);
				}
				else
				{
					return WithinRangeArray(unit->pMovementData->action.goal.unit->type, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, unit->pMovementData->action.goal.unit->curAssociatedSquare.x, unit->pMovementData->action.goal.unit->curAssociatedSquare.y, nextToRangeArray);
				}
			}
			else
			{
				return true;
			}
		}

		bool IsSuitableForBuilding(const gc_ptr<UnitType> type, const gc_ptr<UnitType>& test_type, int build_x, int build_y)
		{
			int m_add_x, m_sub_x;
			int m_add_y, m_sub_y;
			int width_on_map, height_on_map;
			int start_x, start_y, end_x, end_y;
			int changes = 0;
			bool last_val, cur_val;

			if (!SquaresAreWalkable(type, build_x, build_y, SIW_IGNORE_OWN_MOBILE_UNITS))
			{
				return false;
			}

			m_add_x = (int) floor(test_type->widthOnMap / 2.0);
			m_sub_x = (int) floor(test_type->widthOnMap / 2.0 - 0.5);

			m_add_y = (int) floor(test_type->heightOnMap / 2.0);
			m_sub_y = (int) floor(test_type->heightOnMap / 2.0 - 0.5);

			GetTypeUpperLeftCorner(type, build_x, build_y, start_x, start_y);

			end_x = start_x + type->widthOnMap + m_add_x;
			end_y = start_y + type->heightOnMap + m_add_y;

			start_x -= m_sub_x + 1;
			start_y -= m_sub_y + 1;

			width_on_map = test_type->widthOnMap;
			height_on_map = test_type->heightOnMap;

			last_val = SquaresAreWalkable(test_type, start_x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);
			
			for (int x = start_x + width_on_map; x <= end_x; x += width_on_map)
			{
				cur_val = SquaresAreWalkable(test_type, x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			last_val = SquaresAreWalkable(test_type, start_x, end_y, SIW_IGNORE_OWN_MOBILE_UNITS);
			
			for (int x = start_x + width_on_map; x <= end_x; x += width_on_map)
			{
				cur_val = SquaresAreWalkable(test_type, x, end_y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			last_val = SquaresAreWalkable(test_type, start_x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);

			for (int y = start_y + height_on_map; y <= end_y; y += height_on_map)
			{
				cur_val = SquaresAreWalkable(test_type, start_x, y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			last_val = SquaresAreWalkable(test_type, end_x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);

			for (int y = start_y + height_on_map; y <= end_y; y += height_on_map)
			{
				cur_val = SquaresAreWalkable(test_type, end_x, y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			return changes <= 2;
		}
		
		bool IsSuitableForBuilding(const gc_ptr<UnitType>& type, int build_x, int build_y)
		{
			return (IsSuitableForBuilding(type, type->player->unitTypeMap["LargeTank"], build_x, build_y) && 
			        IsSuitableForBuilding(type, type->player->unitTypeMap["SmallTank"], build_x, build_y) &&
			        IsSuitableForBuilding(type, type->player->unitTypeMap["LargeAttackRobot"], build_x, build_y) &&
			        IsSuitableForBuilding(type, type->player->unitTypeMap["SmallAttackRobot"], build_x, build_y));
		}

		int PositionSearch_NumStepsTaken = 0;

		bool GetNearestSuitableAndLightedPosition(const gc_ptr<UnitType>& type, int& x, int& y)
		{
			int num_steps = 1;
			if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y))
			{
				return true;
			}

			for (int j = 0; j < 32; j++)
			{
				for (int i = 0; i < num_steps; i++)
				{
					x++;
					if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y))
					{
						return true;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y++;
					if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y))
					{
						return true;
					}
				}

				num_steps++;
				
				for (int i = 0; i < num_steps; i++)
				{
					x--;
					if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y))
					{
						return true;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y--;
					if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y))
					{
						return true;
					}
				}

				PositionSearch_NumStepsTaken += num_steps * 4 - 2;

				num_steps++;

				if (PositionSearch_NumStepsTaken > 4096 && j > 8)
					return false;
			}
			return false;
		}

		float PercentLightedAtPosition(const gc_ptr<UnitType>& type, int x, int y)
		{
			Uint16** NumLightsOnSquare = pWorld->NumLightsOnSquare;
			int covered = 0, total = 0;
			int start_x, start_y, end_x, end_y;
			int dist_x, dist_y, dist_both;
			float range = type->lightRange;

			start_x = (float) x - type->lightRange < 0 ? 0 : x - (int) type->lightRange;
			start_y = (float) y - type->lightRange < 0 ? 0 : y - (int) type->lightRange;
			end_x = (float) x + type->lightRange >= pWorld->width ? pWorld->width-1 : x + (int) type->lightRange;
			end_y = (float) y + type->lightRange >= pWorld->height ? pWorld->height-1 : y + (int) type->lightRange;

			dist_y = start_y - y;

			for (int ny = start_y; ny < end_y; ny++, dist_y++)
			{
				int nx;
				dist_x = start_x - x;
				dist_both = abs(dist_y) + abs(dist_y);
				for (nx = start_x; nx <= x; nx++, dist_x++, dist_both--)
				{
					if (dist_both < range || Distance2D(dist_x, dist_y) <= range)
					{
						if (NumLightsOnSquare[ny][nx])
						{
							covered++;
						}
						total++;
					}
				}
				for (; nx < end_x; nx++, dist_x++, dist_both++)
				{
					if (dist_both < range || Distance2D(dist_x, dist_y) <= range)
					{
						if (NumLightsOnSquare[ny][nx])
						{
							covered++;
						}
						total++;
					}
				}
			}
			return (float) covered / (float) total;
		}

		bool GetSuitablePositionForLightTower(const gc_ptr<UnitType>& type, int& x, int& y, bool needLighted)
		{
			int num_steps = 1;
			if (!needLighted)
			{
				if (IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
				{
					return true;
				}

				for (int j = 0; j < 32; j++)
				{
					for (int i = 0; i < num_steps; i++)
					{
						x++;
						if (IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y++;
						if (IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					num_steps++;
					
					for (int i = 0; i < num_steps; i++)
					{
						x--;
						if (IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y--;
						if (IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					PositionSearch_NumStepsTaken += num_steps * 4 - 2;

					num_steps++;

					if (PositionSearch_NumStepsTaken > 4096 && j > 8)
						return false;
				}
			}
			else
			{
				if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
				{
					return true;
				}

				for (int j = 0; j < 32; j++)
				{
					for (int i = 0; i < num_steps; i++)
					{
						x++;
						if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y++;
						if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					num_steps++;
					
					for (int i = 0; i < num_steps; i++)
					{
						x--;
						if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y--;
						if (SquaresAreLightedAround(type, x, y) && IsSuitableForBuilding(type, x, y) && SquaresAreWalkable(type, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					PositionSearch_NumStepsTaken += num_steps * 4 - 2;

					num_steps++;
					
					if (PositionSearch_NumStepsTaken > 4096 && j > 8)
						return false;
				}
			}
			return false;
		}

		bool DoesNotBlock(const gc_ptr<Unit>& unit, const gc_ptr<UnitType>& build_type, int build_x, int build_y, int x, int y)
		{
			int start_x_u, start_y_u;
			int start_x_t, start_y_t;
			GetUnitUpperLeftCorner(unit, x, y, start_x_u, start_y_u);
			GetTypeUpperLeftCorner(build_type, build_x, build_y, start_x_t, start_y_t);
			for (int ny = start_y_u; ny < start_y_u + unit->type->heightOnMap; ny++)
			{
				for (int nx = start_x_u; nx < start_x_u + unit->type->widthOnMap; nx++)
				{
					if (nx >= start_x_t && nx < start_x_t + build_type->widthOnMap &&
					    ny >= start_y_t && ny < start_y_t + build_type->heightOnMap)
					{
						return false;
					}
				}
			}
			return true;
		}

		void NearestSquareFromBuildingPlace(const gc_ptr<Unit>& unit, const gc_ptr<UnitType>& build_type, int build_x, int build_y, int &x, int &y)
		{
			int num_steps = 1;
			x = build_x;
			y = build_y;
			if (DoesNotBlock(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
			{
				return;
			}

			for (int j = 0; j < 32; j++)
			{
				for (int i = 0; i < num_steps; i++)
				{
					x++;
					if (DoesNotBlock(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y++;
					if (DoesNotBlock(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				num_steps++;
				
				for (int i = 0; i < num_steps; i++)
				{
					x--;
					if (DoesNotBlock(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y--;
					if (DoesNotBlock(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				num_steps++;
			}
		}
		
		void UpdateSeenSquares(const gc_ptr<Unit>& unit, int x, int y, int operation);

		void Incomplete(const gc_ptr<Unit>& unit)
		{
			UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 0);
			UpdateSeenSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 0);
		}

		void Complete(const gc_ptr<Unit>& unit)
		{
			UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 1);
			UpdateSeenSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 1);
		}

		// returns true if attacker can reach target with an attack
		bool CanReach(const gc_ptr<Unit>& attacker, const gc_ptr<Unit>& target)
		{
			return WouldBeAbleToReach(attacker, attacker->curAssociatedSquare.x, attacker->curAssociatedSquare.y, target);
		}

		bool CanSee(const gc_ptr<Unit>& attacker, const gc_ptr<Unit>& target)
		{
			return WithinRangeArray(target, attacker->curAssociatedSquare.x, attacker->curAssociatedSquare.y, attacker->type->sightRangeArray);
		}

		gc_ptr<Unit> GetNearestUnitInRange(const gc_ptr<Unit>& unit, RangeType rangeType, PlayerState state)
		{
			gc_ptr<Unit> bestUnit = NULL;
			float bestDistance = 1000000.0, distance;
			int max_range = rangeType == RANGE_SIGHT ? (int) ceil(unit->type->sightRange) : (int) ceil(unit->type->attackMaxRange);
			int big_start_x = (unit->curAssociatedSquare.x - max_range - 10) >> bigSquareRightShift;
			int big_start_y = (unit->curAssociatedSquare.y - max_range - 10) >> bigSquareRightShift;
			int big_end_x = (unit->curAssociatedSquare.x + max_range + 10) >> bigSquareRightShift;
			int big_end_y = (unit->curAssociatedSquare.y + max_range + 10) >> bigSquareRightShift;

			if (big_start_y < 0)
				big_start_y = 0;

			if (big_start_x < 0)
				big_start_x = 0;

			if (big_end_y >= bigSquareHeight)
				big_end_y = bigSquareHeight-1;

			if (big_end_x >= bigSquareWidth)
				big_end_x = bigSquareWidth-1;
			
			for (vector<gc_ptr<Player> >::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
			{
				const gc_ptr<Player>& owner = *it;
				if (unit->owner->states[owner->index] & state)
				{
					int owner_index = owner->index;
					for (int y = big_start_y; y <= big_end_y; y++)
					{
						for (int x = big_start_x; x <= big_end_x; x++)
						{
							for (vector<gc_ptr<Unit> >::iterator it_unit = unitsInBigSquaresPerPlayer[owner_index][y][x]->begin(); it_unit != unitsInBigSquaresPerPlayer[owner_index][y][x]->end(); it_unit++)
							{
								const gc_ptr<Unit>& curUnit = *it_unit;
								if (curUnit != unit && curUnit->isDisplayed)
								{
									bool found = false;
									if (rangeType == RANGE_SIGHT)
									{
										if (CanSee(unit, curUnit))
										{
											found = true;
										}
									}
									else if (rangeType == RANGE_ATTACK)
									{
										if (CanReach(unit, curUnit))
										{
											found = true;
										}
									}
									if (found)
									{
										distance = Distance2D(curUnit->pos.x - unit->pos.x, curUnit->pos.y - unit->pos.y);
										if (distance < bestDistance)
										{
											bestUnit = curUnit;
											bestDistance = distance;
										}
									}
								}
							}
						}
					}
				}
			}
			return bestUnit;
		}

		bool UnitIsRendered(const gc_ptr<Unit>& unit, const gc_ptr<Player>& player)
		{
			int start_x, start_y;
			Uint16** NumUnitsSeeingSquare = player->NumUnitsSeeingSquare;
			if (!unit->isDisplayed)
			{
				return false;
			}
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						if (BigSquareIsRendered(x, y) && NumUnitsSeeingSquare[y][x])
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		float GetLightAmountOnUnit(const gc_ptr<Unit>& unit)
		{
			float temp = 0.0;
			int num = 0;
			int start_x, start_y;
			bool is_lighted;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						is_lighted = pWorld->NumLightsOnSquare[y][x] > 0 ? 1 : 0;
						temp += (float) is_lighted;
						num++;
					}
				}
			}
			return temp / (float) num;
		}

		bool UnitIsVisible(const gc_ptr<Unit>& unit, const gc_ptr<Player>& player)
		{
			int start_x, start_y;
			int end_x, end_y;
			Uint16** NumUnitsSeeingSquare = player->NumUnitsSeeingSquare;
			if (!unit->isDisplayed)
			{
				return false;
			}
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			end_x = start_x + unit->type->widthOnMap - 1;
			end_y = start_y + unit->type->heightOnMap - 1;
			for (int y = start_y; y <= end_y; y++)
			{
				Uint16* row = NumUnitsSeeingSquare[y];
				for (int x = start_x; x <= end_x; x++)
				{
					if (row[x])
					{
						return true;
					}
				}
			}
			return false;
		}

		inline bool SquareIsVisible_UnGuarded(const gc_ptr<Player>& player, int x, int y)
		{
			return player->NumUnitsSeeingSquare[y][x];
		}

		bool SquareIsVisible(const gc_ptr<Player>& player, int x, int y)
		{
			if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
			{
				return player->NumUnitsSeeingSquare[y][x];
			}
			else
			{
				return false;
			}
		}

		inline bool MovementTypeCanWalkOnSquare_NoPrecalc(MovementType mType, int x, int y)
		{
			if (x < 0 || y < 0 || x >= pWorld->width || y >= pWorld->height)
			{
				return false;
			}
			int steepness = heightMap->steepness[y][x];
			float height = heightMap->heights[y][x];
			switch (mType)
			{
				case MOVEMENT_HUMAN:
					return steepness < 72 && height > waterLevel;
					break;
				case MOVEMENT_SMALLVEHICLE:
					return steepness < 68 && height > waterLevel;
					break;
				case MOVEMENT_MEDIUMVEHICLE:
					return steepness < 60 && height > waterLevel;
					break;
				case MOVEMENT_LARGEVEHICLE:
					return steepness < 54 && height > waterLevel;
					break;
				case MOVEMENT_BUILDING:
					return steepness < 51 && height > waterLevel && height < unitBuildingMaximumAltitude;
					break;
				default:
					return false;
					break;
			}
		}

		inline bool MovementTypeCanWalkOnSquare_UnGuarded(MovementType mType, int x, int y)
		{
			return movementTypeWithSizeCanWalkOnSquare[0][mType][y][x];
		}

		inline bool MovementTypeCanWalkOnSquares_UnGuarded(MovementType mType, int size, int x, int y)
		{
			return movementTypeWithSizeCanWalkOnSquare[size-1][mType][y][x];
		}

		bool MovementTypeCanWalkOnSquare(MovementType mType, int x, int y)
		{
			if (x < 0 || y < 0 || x >= pWorld->width || y >= pWorld->height)
			{
				return false;
			}
			return MovementTypeCanWalkOnSquare_UnGuarded(mType, x, y);
		}

		bool MovementTypeCanWalkOnSquares(MovementType mType, int size, int x, int y)
		{
			if (x < 0 || y < 0 || x >= pWorld->width || y >= pWorld->height)
			{
				return false;
			}
			return MovementTypeCanWalkOnSquares_UnGuarded(mType, size, x, y);
		}

		bool MovementTypeCanWalkOnSquare_Pathfinding(MovementType mType, int size, int pos_x, int pos_y)
		{
			int start_x, start_y;
			int end_x, end_y;
			if (pos_x < 0 || pos_y < 0 || pos_x >= pWorld->width || pos_y >= pWorld->height)
			{
				return false;
			}
			if (!MovementTypeCanWalkOnSquares_UnGuarded(mType, size+1, pos_x, pos_y))
			{
				return false;
			}
			GetSizeUpperLeftCorner(size, pos_x, pos_y, start_x, start_y);
			end_x = start_x + size - 1;
			end_y = start_y + size - 1;
			for (int y = start_y; y <= end_y; y++)
			{
				for (int x = start_x; x <= end_x; x++)
				{
					gc_ptr<Unit> pUnit = pppElements[y][x];
					if (pUnit && !pUnit->type->isMobile)
					{
						pUnit->usedInAreaMaps = true;
						return false;
					}
				}
			}
			return true;
		}

		inline bool UnitTypeCanWalkOnSquare(const gc_ptr<UnitType>& type, int x, int y)
		{
			return MovementTypeCanWalkOnSquare(type->movementType, x, y);
		}

		inline bool UnitTypeCanWalkOnSquare_UnGuarded(const gc_ptr<UnitType>& type, int x, int y)
		{
			return MovementTypeCanWalkOnSquare_UnGuarded(type->movementType, x, y);
		}

		inline bool UnitTypeCanWalkOnSquares(const gc_ptr<UnitType>& type, int x, int y)
		{
			return MovementTypeCanWalkOnSquares(type->movementType, type->heightOnMap, x, y);
		}

		inline bool UnitTypeCanWalkOnSquares_UnGuarded(const gc_ptr<UnitType>& type, int x, int y)
		{
			return MovementTypeCanWalkOnSquares_UnGuarded(type->movementType, type->heightOnMap, x, y);
		}

		inline bool SquareIsWalkable_Internal(const gc_ptr<Unit>& unit, MovementType movementType, const gc_ptr<Player>& player, int x, int y, int flags)
		{
			bool walkable;
			if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
			{
				walkable = MovementTypeCanWalkOnSquare_NoPrecalc(movementType, x, y);
				if (walkable && (flags & SIW_ALLKNOWING || SquareIsVisible_UnGuarded(player, x, y)))
				{
					gc_ptr<Unit> curUnit = pppElements[y][x];
					if (!curUnit || curUnit == unit)
					{
						return true;
					}
/*					if (flags & SIW_CONSIDER_WAITING && curUnit->isWaiting)
					{
						return false;
					}
					if (flags & SIW_CONSIDER_PUSHED && curUnit->isPushed)
					{
						return false;
					}*/
					if (flags & SIW_IGNORE_MOVING && curUnit->isMoving)
					{
						return true;
					}
					if (flags & SIW_IGNORE_OWN_MOBILE_UNITS && curUnit->owner == player && curUnit->type->isMobile)
					{
						return true;
					}
					return false;
				}
				else
				{
					return walkable;
				}
			}
			return false;
		}

		inline bool SquareIsWalkable_MultipleInternal(const gc_ptr<Unit>& unit, const gc_ptr<Player>& player, int x, int y, int flags)
		{
			if (flags & SIW_ALLKNOWING || SquareIsVisible_UnGuarded(player, x, y))
			{
				gc_ptr<Unit> curUnit = pppElements[y][x];
				if (!curUnit || curUnit == unit)
				{
					return true;
				}
/*				if (flags & SIW_CONSIDER_WAITING && curUnit->isWaiting)
				{
					return false;
				}
				if (flags & SIW_CONSIDER_PUSHED && curUnit->isPushed)
				{
					return false;
				}*/
				if (flags & SIW_IGNORE_MOVING && curUnit->isMoving)
				{
					return true;
				}
				if (flags & SIW_IGNORE_OWN_MOBILE_UNITS && curUnit->owner == player && curUnit->type->isMobile)
				{
					return true;
				}
				return false;
			}
			return true;
		}

		inline bool SquareIsWalkable(const gc_ptr<Unit>& unit, int x, int y, int flags)
		{
			return SquareIsWalkable_Internal(unit, unit->type->movementType, unit->owner, x, y, flags);
		}

		inline bool SquareIsWalkable(MovementType movementType, const gc_ptr<Player>& player, int x, int y, int flags)
		{
			return SquareIsWalkable_Internal(NULL, movementType, player, x, y, flags);
		}

		inline bool SquareIsWalkable(const gc_ptr<Unit>& unit, int x, int y)
		{
			return SquareIsWalkable_Internal(unit, unit->type->movementType, unit->owner, x, y, SIW_DEFAULT);
		}

		bool SquareIsWalkable(const gc_ptr<UnitType>& type, int x, int y, int flags)
		{
			return SquareIsWalkable_Internal(NULL, type->movementType, type->player, x, y, flags);
		}

		inline bool SquareIsWalkable(const gc_ptr<UnitType>& type, int x, int y)
		{
			return SquareIsWalkable_Internal(NULL, type->movementType, type->player, x, y, SIW_DEFAULT);
		}

		bool SquaresAreWalkable_Internal(const gc_ptr<Unit>& unit, MovementType movementType, int width, int height, const gc_ptr<Player>& player, int x, int y, int flags)
		{
			int start_x, start_y;
			int end_x, end_y;
			if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
			{
				if (width == height && width <= 4 && movementTypeWithSizeCanWalkOnSquare[width-1][movementType])
				{
					if (MovementTypeCanWalkOnSquares(movementType, width, x, y))
					{
						GetSizeUpperLeftCorner(width, x, y, start_x, start_y);
						end_x = start_x + width;
						end_y = start_y + height;
						for (int ny = start_y; ny < end_y; ny++)
						{
							for (int nx = start_x; nx < end_x; nx++)
							{
								if (!SquareIsWalkable_MultipleInternal(unit, player, nx, ny, flags))
								{
									return false;
								}
							}
						}
						return true;
					}
				}
				else
				{
					GetWHUpperLeftCorner(width, height, x, y, start_x, start_y);
					end_x = start_x + width;
					end_y = start_y + height;
					for (int ny = start_y; ny < end_y; ny++)
					{
						for (int nx = start_x; nx < end_x; nx++)
						{
							if (!SquareIsWalkable_Internal(unit, movementType, player, nx, ny, flags))
							{
								return false;
							}
						}
					}
					return true;
				}
			}
			return false;
		}

		bool SquaresAreWalkable(const gc_ptr<Unit>& unit, int x, int y, int flags)
		{
			const gc_ptr<UnitType>& type = unit->type;
			return SquaresAreWalkable_Internal(unit, type->movementType, type->widthOnMap, type->heightOnMap, unit->owner, x, y, flags);
		}

		bool SquaresAreWalkable(const gc_ptr<Unit>& unit, int x, int y)
		{
			return SquaresAreWalkable(unit, x, y, SIW_DEFAULT);
		}

		bool SquaresAreWalkable(const gc_ptr<UnitType>& type, int x, int y, int flags)
		{
			return SquaresAreWalkable_Internal(NULL, type->movementType, type->widthOnMap, type->heightOnMap, type->player, x, y, flags);
		}
		
		bool SquaresAreWalkable(MovementType movementType, int width, int height, const gc_ptr<Player>& player, int x, int y, int flags)
		{
			return SquaresAreWalkable_Internal(NULL, movementType, width, height, player, x, y, flags);
		}

		inline bool SquaresAreWalkable(const gc_ptr<UnitType>& type, int x, int y)
		{
			return SquaresAreWalkable(type, x, y, SIW_DEFAULT);
		}

		bool SquareIsLighted(const gc_ptr<Player>& player, int x, int y)
		{
			if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
			{
				if (player->NumUnitsSeeingSquare[y][x])
				{
					return pWorld->NumLightsOnSquare[y][x];
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		inline bool SquareIsLighted_UnGuarded(const gc_ptr<Player>& player, int x, int y)
		{
			if (player->NumUnitsSeeingSquare[y][x])
			{
				return pWorld->NumLightsOnSquare[y][x];
			}
			else
			{
				return false;
			}
		}

		bool SquaresAreLighted(const gc_ptr<UnitType>& type, int x, int y)
		{
			int start_x, start_y;
			int end_x, end_y;
			GetTypeUpperLeftCorner(type, x, y, start_x, start_y);
			end_y = start_y + type->heightOnMap - 1;
			end_x = start_x + type->widthOnMap - 1;
			if (start_x < 0 || start_y < 0 || end_x >= pWorld->width || end_y >= pWorld->height)
			{
				return false;
			}
			for (int ny = start_y; ny <= end_y; ny++)
			{
				for (int nx = start_x; nx <= end_x; nx++)
				{
					if (!SquareIsLighted_UnGuarded(type->player, nx, ny))
					{
						return false;
					}
				}
			}
			return true;
		}

		bool SquaresAreLightedAround(const gc_ptr<UnitType>& type, int x, int y)
		{
			int start_x, start_y;
			GetTypeUpperLeftCorner(type, x, y, start_x, start_y);
			start_x--;
			start_y--;
			for (int ny = start_y; ny < start_y + type->heightOnMap + 2; ny++)
			{
				for (int nx = start_x; nx < start_x + type->widthOnMap + 2; nx++)
				{
					if (!SquareIsLighted(type->player, nx, ny))
					{
						return false;
					}
				}
			}
			return true;
		}

		bool SquareIsGoal(const gc_ptr<Unit>& unit, int x, int y, bool use_internal)
		{
			gc_ptr<Unit> target = NULL;
			IntPosition pos;
			ActionArguments args;
			
			if (!use_internal)
			{
				target = unit->pMovementData->action.goal.unit;
				pos    = unit->pMovementData->action.goal.pos;
				args   = unit->pMovementData->action.args;
			}
			else
			{
				target = unit->pMovementData->_action.goal.unit;
				pos    = unit->pMovementData->_action.goal.pos;
				args   = unit->pMovementData->_action.args;
			}
		
			if(target)
			{
				if (unit->pMovementData->action.action == AI::ACTION_ATTACK || unit->pMovementData->action.action == AI::ACTION_MOVE_ATTACK_UNIT)
				{
					return WouldBeAbleToReach(unit, x, y, target);
				}
				else if (unit->pMovementData->action.action == AI::ACTION_FOLLOW)
				{
					return WithinRangeArray(target->type, x, y, target->curAssociatedSquare.x, target->curAssociatedSquare.y, nextToRangeArray);
				}
				else if (unit->pMovementData->action.action == AI::ACTION_BUILD)
				{
					if (args.unitType)
					{
						return WithinRangeArray(args.unitType, x, y, pos.x, pos.y, nextToRangeArray);
					}
					else
					{
						return WithinRangeArray(target->type, x, y, target->curAssociatedSquare.x, target->curAssociatedSquare.y, nextToRangeArray);
					}
				}
				else
				{
					return (x == pos.x) && (y == pos.y);
				}
			}
			else
			{
				return (x == pos.x) && (y == pos.y);
			}
		}		

		// operation is 0 for removing seen squares, 1 for adding seen squares.
		void UpdateSeenSquares(const gc_ptr<Unit>& unit, int x, int y, int operation)
		{
			int start_x, start_y, end_x, end_y;
			Uint16** NumUnitsSeeingSquare = unit->owner->NumUnitsSeeingSquare;
			const gc_ptr<RangeScanlines>& rangeScanlines = unit->type->sightRangeScanlines;

			if (unit->owner->isRemote)
			{
				// Do not calculate seen squares for units not controlled by this client/server
				return;
			}
			
			if (operation == 0 && unit->hasSeen == false)
			{
				return;
			}
			else if (operation == 1 && unit->hasSeen == true)
			{
				cout << "SEEN SQUARES MANAGEMENT WARNING: Attempted to add seen squares twice" << endl;
				return;
			}

			unit->hasSeen = operation == 1 ? true : false;

			int offset = rangeScanlines->yOffset;
			int size = rangeScanlines->height;
			start_y = y - offset < 0 ? offset - y : 0;
			end_y = y + offset >= pWorld->height ? size - offset + pWorld->height - y - 1 : size;
			for (int ry = start_y; ry < end_y; ry++)
			{
				int ny = ry + y - offset;
				start_x = x + rangeScanlines->scanlines[ry].startX;
				end_x = x + rangeScanlines->scanlines[ry].endX;
				start_x = start_x < 0 ? 0 : start_x;
				end_x = end_x >= pWorld->width ? pWorld->width : end_x;
				if (operation)
				{
					for (int nx = start_x; nx < end_x; nx++)
					{
						NumUnitsSeeingSquare[ny][nx]++;
						if (NumUnitsSeeingSquare[ny][nx] == 1 && pppElements[ny][nx] && pppElements[ny][nx]->owner != unit->owner)
						{
							// A unit that might have been hidden before has been spotted.
							// Update the lastSeenPosition in the spotted unit of the player
							// of the spotting unit correctly.
							int player_index = unit->owner->index;
							gc_ptr<Unit> rev_unit = pppElements[ny][nx];

							rev_unit->lastSeenPositions[player_index] = rev_unit->curAssociatedSquare;
						}
					}
				}
				else
				{
					for (int nx = start_x; nx < end_x; nx++)
					{
						NumUnitsSeeingSquare[ny][nx]--;
					}
				}
			}
		}

		// operation is 0 for removing lighted squares, 1 for adding lighted squares.
		void UpdateLightedSquares(const gc_ptr<Unit>& unit, int x, int y, int operation)
		{
			int start_x, start_y, end_x, end_y;
			Uint16** NumLightsOnSquare = pWorld->NumLightsOnSquare;
			const gc_ptr<RangeScanlines>& rangeScanlines = unit->type->lightRangeScanlines;

			if (unit->type->lightRange < 1e-3)
			{
				return;
			}

			if (operation == 1 && unit->lightState == LIGHT_OFF)
			{
				return;
			}
			else if (operation == 0 && !unit->isLighted)
			{
				return;
			}

			if (operation == 1)
			{
				if (unit->isLighted)
				{
					cout << "LIGHTED SQUARES MANAGEMENT WARNING: Attempt to light the squares of a unit twice" << endl;
					return;
				}
				unit->isLighted = true;
			}
			else if (operation == 0)
			{
				unit->isLighted = false;
			}

			int offset = rangeScanlines->yOffset;
			int size = rangeScanlines->height;
			start_y = y - offset < 0 ? offset - y : 0;
			end_y = y + offset >= pWorld->height ? size - offset + pWorld->height - y - 1 : size;
			for (int ry = start_y; ry < end_y; ry++)
			{
				int ny = ry + y - offset;
				start_x = x + rangeScanlines->scanlines[ry].startX;
				end_x = x + rangeScanlines->scanlines[ry].endX;
				start_x = start_x < 0 ? 0 : start_x;
				end_x = end_x >= pWorld->width ? pWorld->width : end_x;

				if (operation == 1)
				{
					for (int nx = start_x; nx < end_x; nx++)
					{
						NumLightsOnSquare[ny][nx]++;
					}
				}
				else
				{
					for (int nx = start_x; nx < end_x; nx++)
					{
						NumLightsOnSquare[ny][nx]--;
					}
				}
			}
		}

		set<gc_ptr<Unit> > ScheduledBigSquareUpdates;

		bool SetAssociatedSquares(const gc_ptr<Unit>& unit, int new_x, int new_y)
		{
			int start_x, start_y, end_x, end_y;
			if (!SquaresAreWalkable(unit, new_x, new_y, SIW_ALLKNOWING))
			{
				return false;
			}
			
			UpdateSeenSquares(unit, new_x, new_y, 1); // add new
			UpdateLightedSquares(unit, new_x, new_y, 1); // add new
			
			unit->curAssociatedSquare.x = new_x;
			unit->curAssociatedSquare.y = new_y;

			GetUnitUpperLeftCorner(unit, new_x, new_y, start_x, start_y);
			end_x = start_x + unit->type->widthOnMap - 1;
			end_y = start_y + unit->type->heightOnMap - 1;
			for (int y = start_y; y <= end_y; y++)
			{
				for (int x = start_x; x <= end_x; x++)
				{
					pppElements[y][x] = unit;
				}
			}

			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				if (UnitIsVisible(unit, pWorld->vPlayers.at(i)))
				{
					unit->lastSeenPositions[i].x = new_x;
					unit->lastSeenPositions[i].y = new_y;
				}
			}
			
			int old_big_x = unit->curAssociatedBigSquare.x;
			int old_big_y = unit->curAssociatedBigSquare.y;
			int new_big_x = new_x >> bigSquareRightShift;
			int new_big_y = new_y >> bigSquareRightShift;

			if (old_big_x != new_big_x || old_big_y != new_big_y)
			{
				ScheduledBigSquareUpdates.insert(unit);
			}

			return true;
		}

		void ApplyScheduledBigSquareUpdates()
		{
			for (set<gc_ptr<Unit> >::iterator it = ScheduledBigSquareUpdates.begin(); it != ScheduledBigSquareUpdates.end(); it++)
			{
				const gc_ptr<Unit>& unit = *it;
				int old_big_x = unit->curAssociatedBigSquare.x;
				int old_big_y = unit->curAssociatedBigSquare.y;
				int new_big_x = unit->curAssociatedSquare.x >> bigSquareRightShift;
				int new_big_y = unit->curAssociatedSquare.y >> bigSquareRightShift;

				if (old_big_x > -1 && old_big_y > -1)
				{
					vector<gc_ptr<Unit> > *unit_vector = unitsInBigSquaresPerPlayer[unit->owner->index][old_big_y][old_big_x];
					for (vector<gc_ptr<Unit> >::iterator it = unit_vector->begin(); it != unit_vector->end(); it++)
					{
						if (*it == unit)
						{
							unit_vector->erase(it);
							break;
						}
					}
					unit_vector = unitsInBigSquares[old_big_y][old_big_x];
					for (vector<gc_ptr<Unit> >::iterator it = unit_vector->begin(); it != unit_vector->end(); it++)
					{
						if (*it == unit)
						{
							unit_vector->erase(it);
							break;
						}
					}
				}
				unitsInBigSquaresPerPlayer[unit->owner->index][new_big_y][new_big_x]->push_back(unit);
				unitsInBigSquares[new_big_y][new_big_x]->push_back(unit);
				unit->curAssociatedBigSquare.x = new_big_x;
				unit->curAssociatedBigSquare.y = new_big_y;
			}
			ScheduledBigSquareUpdates.clear();
		}

		void RemoveUnitFromBigSquare(const gc_ptr<Unit>& unit)
		{
			if (unit->curAssociatedBigSquare.y > -1)
			{
				vector<gc_ptr<Unit> > *unit_vector = unitsInBigSquaresPerPlayer[unit->owner->index][unit->curAssociatedBigSquare.y][unit->curAssociatedBigSquare.x];
				for (vector<gc_ptr<Unit> >::iterator it = unit_vector->begin(); it != unit_vector->end(); it++)
				{
					if (*it == unit)
					{
						unit_vector->erase(it);
						break;
					}
				}
				unit_vector = unitsInBigSquares[unit->curAssociatedBigSquare.y][unit->curAssociatedBigSquare.x];
				for (vector<gc_ptr<Unit> >::iterator it = unit_vector->begin(); it != unit_vector->end(); it++)
				{
					if (*it == unit)
					{
						unit_vector->erase(it);
						break;
					}
				}
			}

		}

		void DeleteAssociatedSquares(const gc_ptr<Unit>& unit, int old_x, int old_y)
		{
			int start_x, start_y, end_x, end_y;
			UpdateSeenSquares(unit, old_x, old_y, 0); // remove old
			UpdateLightedSquares(unit, old_x, old_y, 0); // remove old

			GetUnitUpperLeftCorner(unit, old_x, old_y, start_x, start_y);
			end_x = start_x + unit->type->widthOnMap - 1;
			end_y = start_y + unit->type->heightOnMap - 1;
			for (int y = start_y; y <= end_y; y++)
			{
				for (int x = start_x; x <= end_x; x++)
				{
					pppElements[y][x] = NULL;
				}
			}

			return;
		}

		// updates pppElements according to how the unit has moved on the grid
		bool UpdateAssociatedSquares(const gc_ptr<Unit>& unit, int new_x, int new_y, int old_x, int old_y)
		{
			if (!SquaresAreWalkable(unit, new_x, new_y, SIW_ALLKNOWING))
			{
				return false;
			}
			
			if (unit->curAssociatedSquare.x != old_x || unit->curAssociatedSquare.y != old_y)
			{
				cout << "ASSOCIATED SQUARES MANAGEMENT WARNING: Attempted to delete squares in another place than where they were once added" << endl;
				old_x = unit->curAssociatedSquare.x;
				old_y = unit->curAssociatedSquare.y;
			}

			DeleteAssociatedSquares(unit, old_x, old_y);
			return SetAssociatedSquares(unit, new_x, new_y);
		}

		void SetLightState(const gc_ptr<Unit>& unit, LightState lightState)
		{
			UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 0); // del old
			unit->lightState = lightState;
			UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 1); // add new
		}

		int GetTraversalTime(const gc_ptr<Unit>& unit, int x, int y, int dx, int dy)
		{
			int time = traversalTimeBySize[unit->type->widthOnMap-1][y+dy][x+dx];
			if (dy & dx)
			{
				time += time >> 1;
			}
			return time;
		}

		int GetTraversalTimeAdjusted(const gc_ptr<Unit>& unit, int x, int y, int dx, int dy)
		{
			int time = GetTraversalTime(unit, x, y, dx, dy);
/*			if (!SquaresAreLighted(unit->type, unit->owner, x+dx, y+dy))
			{
				time = time << 1;
			}*/
			return time;
		}

		void CheckPrecomputedArrays(const gc_ptr<UnitType>& type)
		{
			if (type->isMobile)
			{
				int j = type->widthOnMap-1;
				int i = type->movementType;
				
				if (!movementTypeWithSizeCanWalkOnSquare[j][i])
				{
					movementTypeWithSizeCanWalkOnSquare[j][i] = new char*[pWorld->height];
					for (int y = 0; y < pWorld->height; y++)
					{
						movementTypeWithSizeCanWalkOnSquare[j][i][y] = new char[pWorld->width];
						for (int x = 0; x < pWorld->width; x++)
						{
							int start_x, start_y;
							GetSizeUpperLeftCorner(j+1, x, y, start_x, start_y);
							for (int y2 = start_y; y2 <= start_y + j; y2++)
							{
								for (int x2 = start_x; x2 <= start_x + j; x2++)
								{
									if (!MovementTypeCanWalkOnSquare_NoPrecalc((MovementType)i, x2, y2))
									{
										goto not_walkable;
									}
								}
							}
							movementTypeWithSizeCanWalkOnSquare[j][i][y][x] = 1;
							continue;

							not_walkable:
							movementTypeWithSizeCanWalkOnSquare[j][i][y][x] = 0;
						}
					}
				}

				if (!traversalTimeBySize[j])
				{
					traversalTimeBySize[j] = new char*[pWorld->height];
					for (int y = 0; y < pWorld->height; y++)
					{
						traversalTimeBySize[j][y] = new char[pWorld->width];
						for (int x = 0; x < pWorld->width; x++)
						{
							int start_x, start_y;
							int steepness = 0, num = 0;
							GetSizeUpperLeftCorner(j+1, x, y, start_x, start_y);
							for (int y2 = start_y; y2 <= start_y + j; y2++)
							{
								for (int x2 = start_x; x2 <= start_x + j; x2++)
								{
									if (x2 >= 0 && y2 >= 0 && x2 < pWorld->width && y2 < pWorld->height)
									{
										steepness += heightMap->steepness[y2][x2];
										num++;
									}
								}
							}
							steepness /= num;

							if (steepness > 255)
								steepness = 255;

							traversalTimeBySize[j][y][x] = char(10 + (steepness >> 1));
						}
					}
				}
			}
		}

		void InitUnitSquares()
		{
			movementTypeWithSizeCanWalkOnSquare = new char***[4];
			for (int j = 0; j < 4; j++)
			{
				movementTypeWithSizeCanWalkOnSquare[j] = new char**[Game::Dimension::MOVEMENT_TYPES_NUM];
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					movementTypeWithSizeCanWalkOnSquare[j][i] = NULL;
				}
			}
			
			traversalTimeBySize = new char**[4];
			for (int j = 0; j < 4; j++)
			{
				traversalTimeBySize[j] = NULL;
			}
			
			bigSquareWidth = (pWorld->width>>bigSquareRightShift)+1;
			bigSquareHeight = (pWorld->height>>bigSquareRightShift)+1;

			unitsInBigSquaresPerPlayer = new vector<gc_ptr<Unit> >***[pWorld->vPlayers.size()];
			for (unsigned i = 0; i < pWorld->vPlayers.size(); i++)
			{
				unitsInBigSquaresPerPlayer[i] = new vector<gc_ptr<Unit> >**[bigSquareHeight];
				for (int y = 0; y < bigSquareHeight; y++)
				{
					unitsInBigSquaresPerPlayer[i][y] = new vector<gc_ptr<Unit> >*[bigSquareWidth];
					for (int x = 0; x < bigSquareWidth; x++)
					{
						unitsInBigSquaresPerPlayer[i][y][x] = new vector<gc_ptr<Unit> >;
					}
				}
			}
			
			unitsInBigSquares = new vector<gc_ptr<Unit> >**[bigSquareHeight];
			for (int y = 0; y < bigSquareHeight; y++)
			{
				unitsInBigSquares[y] = new vector<gc_ptr<Unit> >*[bigSquareWidth];
				for (int x = 0; x < bigSquareWidth; x++)
				{
					unitsInBigSquares[y][x] = new vector<gc_ptr<Unit> >;
				}
			}
			
			nextToRangeArray = GenerateRangeArray(1.5, 0);

			for (vector<gc_ptr<Player> >::iterator player = pWorld->vPlayers.begin(); player != pWorld->vPlayers.end(); player++)
			{
				if ((*player)->unitTypeMap["LargeTank"])
				{
					CheckPrecomputedArrays((*player)->unitTypeMap["LargeTank"]);
					CheckPrecomputedArrays((*player)->unitTypeMap["SmallTank"]);
					CheckPrecomputedArrays((*player)->unitTypeMap["LargeAttackRobot"]);
					CheckPrecomputedArrays((*player)->unitTypeMap["SmallAttackRobot"]);
				}
			}
		}

	}
}
