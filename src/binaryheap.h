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

typedef struct
{
	Uint32 *scores;
	BINARY_HEAP_DATATYPE *data;
	int *positions;
	int num_items;
} binary_heap_t;

#define BINARY_HEAP_PARENT(x) (x>>1)
#define BINARY_HEAP_CHILD1(x) (x<<1)
#define BINARY_HEAP_CHILD2(x) ((x<<1)+1)

binary_heap_t *binary_heap_create(Uint32 *scores, BINARY_HEAP_DATATYPE *data, int *positions)
{
	binary_heap_t *heap = (binary_heap_t*) malloc(sizeof(binary_heap_t));
	heap->scores = scores-1;
	heap->data = data;
	heap->positions = positions;
	heap->num_items = 0;
	return heap;
}

void binary_heap_destroy(binary_heap_t *heap)
{
	free(heap);
}

void binary_heap_push_item(binary_heap_t *heap, BINARY_HEAP_DATATYPE new_data, Uint32 score)
{
	int pos, parent_pos, temp_score;
	BINARY_HEAP_DATATYPE temp_data;
	BINARY_HEAP_DATATYPE *data = heap->data;
	Uint32 *scores = heap->scores;
	int *positions = heap->positions;
	heap->num_items++;
	pos = heap->num_items;
	scores[pos] = score;
	data[pos] = new_data;
	positions[new_data] = pos;
	while (pos != 1 && scores[pos] < scores[BINARY_HEAP_PARENT(pos)])
	{
		parent_pos = BINARY_HEAP_PARENT(pos);
		temp_score = scores[parent_pos];
		temp_data = data[parent_pos];
		scores[parent_pos] = scores[pos];
		data[parent_pos] = data[pos];
		scores[pos] = temp_score;
		data[pos] = temp_data;
		positions[data[parent_pos]] = parent_pos;
		positions[temp_data] = pos;
		pos = parent_pos;
	}
}

BINARY_HEAP_DATATYPE binary_heap_pop_item(binary_heap_t *heap, BINARY_HEAP_DATATYPE def)
{
	BINARY_HEAP_DATATYPE ret = def;
	int curpos = 1, lastpos;
	int temp_score;
	BINARY_HEAP_DATATYPE temp_data;
	BINARY_HEAP_DATATYPE *data = heap->data;
	Uint32 *scores = heap->scores;
	int *positions = heap->positions;

	if (heap->num_items)
	{
		ret = data[curpos];
		data[curpos] = data[heap->num_items];
		scores[curpos] = scores[heap->num_items];
		heap->num_items--;
		if (heap->num_items)
		{
			while (1)
			{
				int child1 = BINARY_HEAP_CHILD1(curpos);
				int child2 = BINARY_HEAP_CHILD2(curpos);
				lastpos = curpos;
				if (child2 <= heap->num_items)
				{
					if (scores[child2] >= scores[child1])
					{
						curpos = child1;
					}
					else
					{
						curpos = child2;
					}
					if (scores[curpos] >= scores[lastpos])
					{
						curpos = lastpos;
					}
				}
				else if (child1 <= heap->num_items)
				{
					if (scores[curpos] >= scores[child1])
					{
						curpos = child1;
					}
				}
				if (curpos != lastpos)
				{
					temp_score = scores[curpos];
					temp_data = data[curpos];
					scores[curpos] = scores[lastpos];
					data[curpos] = data[lastpos];
					scores[lastpos] = temp_score;
					data[lastpos] = temp_data;
					positions[data[curpos]] = curpos;
					positions[temp_data] = lastpos;
				}
				else
				{
					break;
				}
			}
		}
	}
	return ret;
}

void binary_heap_lower_item_score(binary_heap_t *heap, BINARY_HEAP_DATATYPE altered_data, Uint32 score)
{
	int pos, parent_pos, temp_score;
	BINARY_HEAP_DATATYPE temp_data;
	BINARY_HEAP_DATATYPE *data = heap->data;
	Uint32 *scores = heap->scores;
	int *positions = heap->positions;
	pos = positions[altered_data];

	scores[pos] = score;
	while (pos != 1 && scores[pos] < scores[BINARY_HEAP_PARENT(pos)])
	{
		parent_pos = BINARY_HEAP_PARENT(pos);
		temp_score = scores[parent_pos];
		temp_data = data[parent_pos];
		scores[parent_pos] = scores[pos];
		data[parent_pos] = data[pos];
		scores[pos] = temp_score;
		data[pos] = temp_data;
		positions[data[parent_pos]] = parent_pos;
		positions[temp_data] = pos;
		pos = parent_pos;
	}
}

void binary_heap_pop_all(binary_heap_t *heap)
{
	heap->num_items = 0;
}

