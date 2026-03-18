//-----------------------------------------------------------------------------
// AI Bridge Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//
// AIBridgeManager - Connects external AI/LLM services to Torque3D's
// AIPlayer system. Runs a TCP listener that accepts JSON commands from
// external processes (Python, Node.js, etc.) and translates them into
// engine actions (movement, aiming, object interaction, browser control).
//
// Protocol (JSON over TCP, newline-delimited):
//
// → { "action": "move", "id": 1234, "x": 100, "y": 200, "z": 30 }
// → { "action": "aim", "id": 1234, "targetId": 5678 }
// → { "action": "fire", "id": 1234, "trigger": 0, "state": true }
// → { "action": "browse", "id": 1234, "url": "https://example.com" }
// → { "action": "js", "id": 1234, "code": "document.title" }
// → { "action": "say", "id": 1234, "message": "Hello!" }
// → { "action": "spawn", "datablock": "AIPlayerData", "x": 0, "y": 0, "z": 100 }
// → { "action": "query", "id": 1234 }  // returns world state
// → { "action": "exec", "script": "any TorqueScript here" }
//
// ← { "type": "state", "id": 1234, "pos": [x,y,z], "aim": [x,y,z], ... }
// ← { "type": "event", "id": 1234, "event": "reachDestination" }
// ← { "type": "result", "success": true, "data": { ... } }
//-----------------------------------------------------------------------------

#ifndef _AI_BRIDGE_MANAGER_H_
#define _AI_BRIDGE_MANAGER_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class AIPlayer;
class BrowserTexture;

/// Singleton that bridges external AI services to the Torque3D engine.
class AIBridgeManager : public SimObject
{
   typedef SimObject Parent;

public:
   DECLARE_CONOBJECT(AIBridgeManager);

   AIBridgeManager();
   virtual ~AIBridgeManager();

   static void initPersistFields();

   bool onAdd() override;
   void onRemove() override;

   /// Access the singleton.
   static AIBridgeManager* get();

   /// Start listening for AI connections on the specified port.
   bool startListener(U32 port);

   /// Stop the listener.
   void stopListener();

   /// Process incoming commands. Called once per frame.
   void process();

   /// Process a single JSON command string.
   /// @return JSON response string.
   const char* processCommand(const char *jsonCmd);

   /// Register an AIPlayer as AI-controllable.
   void registerAgent(U32 agentId, AIPlayer *player);
   void unregisterAgent(U32 agentId);

   /// Bind a BrowserTexture to an agent (their "screen").
   void bindBrowser(U32 agentId, BrowserTexture *browser);

   /// Get world state for an agent as JSON.
   const char* getAgentState(U32 agentId);

   /// Get a summary of all agents as JSON.
   const char* getAllAgentsState();

   /// Is the bridge active?
   bool isActive() const { return mListening; }

protected:
   static AIBridgeManager *smInstance;

   /// Port to listen on.
   U32 mPort;

   /// Are we actively listening?
   bool mListening;

   /// Agent registry: maps agentId → AIPlayer.
   struct AgentEntry
   {
      U32 id;
      AIPlayer *player;
      BrowserTexture *browser;

      AgentEntry() : id(0), player(NULL), browser(NULL) {}
   };

   Vector<AgentEntry> mAgents;

   /// Find an agent by ID.
   AgentEntry* _findAgent(U32 id);

   /// Handle individual command types.
   const char* _handleMove(U32 id, F32 x, F32 y, F32 z);
   const char* _handleAim(U32 id, F32 x, F32 y, F32 z);
   const char* _handleAimObject(U32 id, U32 targetId);
   const char* _handleFire(U32 id, U32 trigger, bool state);
   const char* _handleBrowse(U32 id, const char *url);
   const char* _handleJavaScript(U32 id, const char *code);
   const char* _handleSpawn(const char *datablock, F32 x, F32 y, F32 z);
   const char* _handleQuery(U32 id);
   const char* _handleExec(const char *script);
};

#endif // _AI_BRIDGE_MANAGER_H_
