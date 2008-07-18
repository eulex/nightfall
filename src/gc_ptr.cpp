
#include "gc_ptr.h"

void gc_marker_base::insert()
{
	SDL_LockMutex(mutex);

	marked[mark].insert(this);

	SDL_UnlockMutex(mutex);
}

void gc_marker_base::shade()
{
	if (tempMark == MARK_WHITE)
	{
		marked_temp[MARK_GRAY].insert(this);
		marked_temp[MARK_WHITE].erase(this);
		tempMark = MARK_GRAY;
	}
}

gc_marker_base::gc_marker_base(Mark mark, int refs) : mark(mark), tempMark(MARK_WHITE), refs(refs)
{
	insert();
}
		
void gc_marker_base::increfs()
{
	SDL_LockMutex(mutex);
	assert(refs >= 0);
	if (refs == 0)
	{
		marked[MARK_WHITE].erase(this);
		marked[MARK_GRAY].insert(this);

		mark = MARK_GRAY;
	}
	refs++;
	SDL_UnlockMutex(mutex);
}

void gc_marker_base::decrefs()
{
	SDL_LockMutex(mutex);
	assert(refs > 0);
	refs--;
	if (refs == 0 && mark == MARK_GRAY)
	{
		marked[MARK_GRAY].erase(this);
		marked[MARK_WHITE].insert(this);

		mark = MARK_WHITE;
	}
	SDL_UnlockMutex(mutex);
}

void gc_marker_base::sweep()
{
	if (!startedCollecting)
	{
		SDL_LockMutex(mutex);

		marked_temp[MARK_WHITE] = marked[MARK_WHITE];
		marked_temp[MARK_GRAY] = marked[MARK_GRAY];

		SDL_UnlockMutex(mutex);

		startedCollecting = true;

	}

	MarkerSet& white = marked_temp[MARK_WHITE];
	for (MarkerSet::iterator it = white.begin(); it != white.end(); it++)
	{
		(*it)->tempMark = MARK_WHITE;
	}

	MarkerSet& gray = marked_temp[MARK_GRAY];
	while (gray.size())
	{
		gc_marker_base* m2 = *gray.begin();
		gray.erase(gray.begin());
		m2->tempMark = MARK_GRAY;
		m2->blacken();
	}

	if (!gray.size())
	{
		MarkerSet& white = marked_temp[MARK_WHITE];

		int amount = 0;

		std::map<std::string, int> numPerSize;

		for (MarkerSet::iterator it = white.begin(); it != white.end(); it++)
		{
			std::string name = (*it)->name();
			amount += (*it)->size();
			if (numPerSize.find(name) == numPerSize.end())
			{
				numPerSize[name] = 1;
			}
			else
			{
				numPerSize[name]++;
			}
			delete *it;
			(*it)->dispose();
		}

		SDL_LockMutex(mutex);
		MarkerSet& rwhite = marked[MARK_WHITE];

		std::cout << "Swept " << amount << " bytes in " << white.size() << " objects out of " << rwhite.size() << ". Gray: " << marked[MARK_GRAY].size() << std::endl;
		for (std::map<std::string, int>::iterator it = numPerSize.begin(); it != numPerSize.end(); it++)
		{
			std::cout << it->first << ": " << it->second << std::endl;
		}

		for (MarkerSet::iterator it = white.begin(); it != white.end(); it++)
		{
			rwhite.erase(rwhite.find(*it));
		}
		SDL_UnlockMutex(mutex);

		startedCollecting = false;
	}
}

gc_marker_base::MarkerSet gc_marker_base::marked[gc_marker_base::MARK_NUM];
gc_marker_base::MarkerSet gc_marker_base::marked_temp[gc_marker_base::MARK_NUM];

SDL_mutex* gc_marker_base::mutex = SDL_CreateMutex();

gc_marker_base::CollectStep gc_marker_base::collectStep = gc_marker_base::COLLECTSTEP_NOTSTARTED;
bool gc_marker_base::startedCollecting = false;

