/****************************************************************************************
 *
 * File:
 * 		Node.h
 *
 * Purpose:
 *		The base passive node.
 *
 * Developer Notes:
 *
 *
 ***************************************************************************************/

#pragma once

class MessageBus;

#include "MessageBus/NodeIDs.h"
#include "MessageBus/MessageBus.h"
#include "MessageBus/Message.h"
#include "DataBase/DBHandler.h"

class Node {
public:
	Node(NodeID id, MessageBus& msgBus) : m_MsgBus(msgBus), m_NodeID(id)
	{ }

	Node(NodeID id, MessageBus& msgBus, DBHandler& dbh) : m_MsgBus(msgBus), m_NodeID(id), m_DbHandler(dbh)
	{ }

	virtual ~Node(){};

	///----------------------------------------------------------------------------------
 	/// This function should attempt to initialise or setup any resources it may need to
 	/// function. If this was successful this function should return true.
 	///
 	///----------------------------------------------------------------------------------
	virtual bool init() = 0;

	///----------------------------------------------------------------------------------
 	/// Called by the MessageBus when it has a message the node might be interested in.
 	/// A node should register for messages it wants to receive using
 	/// MessageBus::registerNode(Node*, MessageType)
 	///
 	///----------------------------------------------------------------------------------
	virtual void processMessage(const Message* message) = 0;

	///----------------------------------------------------------------------------------
	/// This function should get the last configuring values from the DataBase
	/// That enables the user to change this values by the web interface
	/// which changes the behaviour of the Node and thus the sailing robot behaviour.
	///
	///----------------------------------------------------------------------------------
	virtual void updateFromDB(){};

	NodeID nodeID() { return m_NodeID; }
protected:
	MessageBus& m_MsgBus;
	DBHandler& m_DbHandler;
private:
	NodeID m_NodeID;
};
