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
#ifndef NETWORKING_H
#define NETWORKING_H

#ifdef DEBUG_DEP
#warning "networking.h"
#endif

#include "networking-pre.h"

#include "unit-pre.h"
#include "dimension-pre.h"
#include "aibase-pre.h"
#include "ainode.h"
#include "circularbuffer.h"
#include "action-pre.h"
#include "unit-pre.h"

namespace Game
{
	namespace Networking
	{
		struct NetActionData
		{
			Uint16 unit_id;
			Uint16 x;
			Uint16 y;
			Uint16 goalunit_id;
			AI::UnitAction action;
			Uint8 rot;
			Uint16 arg;
			Uint32 valid_at_frame;
		};

		struct NetPath
		{
			Uint16 unit_id;
			AI::Node *pStart;
			AI::Node *pGoal;
			Uint32 valid_at_frame;
		};
		
		struct NetCreate
		{
			Uint16 unittype_id;
			Uint16 owner_id;
			Uint16 x;
			Uint16 y;
			Uint8 rot;
			Uint32 valid_at_frame;
		};
		
		struct NetDamage
		{
			Uint16 unit_id;
			Uint32 damage; // x100
			Uint32 valid_at_frame;
		};

		struct NetSell
		{
			Uint16 owner_id;
			Uint32 amount;
			Uint32 valid_at_frame;
		};
		
		void PrepareAction(const gc_ptr<Dimension::Unit>& unit, const gc_ptr<Dimension::Unit>& target, int x, int y, AI::UnitAction action, const Dimension::ActionArguments& args, float rotation);
		void PreparePath(const gc_ptr<Dimension::Unit>& unit, AI::Node* pStart, AI::Node* pGoal);
		void PrepareCreation(const gc_ptr<Dimension::UnitType>& unittype, int x, int y, float rot);
		void PrepareDamaging(const gc_ptr<Dimension::Unit>& unit, float damage);
		void PrepareSell(const gc_ptr<Dimension::Player>& owner, int amount);

		typedef Uint8 BUFFER;

		struct Chunk
		{
			Uint8 id[4];
			Uint16 length;
			BUFFER* data;
		};

		struct Packet
		{
			Uint8 id[4];
			BUFFER* frame;
			Uint16 frameLength;
			Uint16 numChunks;
			Chunk** chunks;
			int references;
			int node;

			Packet() { node = 0; }
		};

		int _networkServerSendThread(void* arg);
		int _networkServerRecvThread(void* arg);
		int _networkSendThread(void* arg);
		int _networkRecvThread(void* arg);

		extern NETWORKTYPE networkType;

#ifdef CHECKSUM_DEBUG
		extern CircularBuffer checksum_output;
#endif

		void InitNetwork();
		int StartNetwork(NETWORKTYPE type);
		
		Uint32 ChecksumPacket(BUFFER* rawdata, int len);
		Packet* ProcessPacket(BUFFER* rawdata, int rawlen);

		void AppendDataToPacket(BUFFER*& pDest, void* pSrc, int len);
		int CreatePacket(Uint8* packet, Packet* data);
		void DeletePacket(Packet* data);
		
		Packet *PopReceivedPacket();
		void PushPacketToSend(Packet *packet);
		unsigned int QueueSize();
		
		void InitIngameNetworking();
	}
}

#ifdef DEBUG_DEP
#warning "networking.h-end"
#endif

#endif
