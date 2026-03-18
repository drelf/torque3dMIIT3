//-----------------------------------------------------------------------------
// AI Bridge Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//-----------------------------------------------------------------------------

#include "aiBridge/aiBridgeManager.h"
#include "T3D/aiPlayer.h"
#include "browserRender/browserTexture.h"
#include "console/console.h"
#include "console/script.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "sim/netObject.h"
#include "math/mPoint3.h"

//-----------------------------------------------------------------------------

AIBridgeManager *AIBridgeManager::smInstance = NULL;

IMPLEMENT_CONOBJECT(AIBridgeManager);

ConsoleDocClass(AIBridgeManager,
   "@brief Bridges external AI/LLM services to Torque3D.\n\n"
   "AIBridgeManager listens on a TCP port for JSON commands and translates "
   "them into engine actions on AIPlayer instances and BrowserTextures.\n\n"
   "@ingroup AIBridge\n"
);

//-----------------------------------------------------------------------------

AIBridgeManager::AIBridgeManager()
   : mPort(9090),
     mListening(false)
{
}

AIBridgeManager::~AIBridgeManager()
{
   if (smInstance == this)
      smInstance = NULL;
}

AIBridgeManager* AIBridgeManager::get()
{
   if (!smInstance)
   {
      smInstance = new AIBridgeManager();
      smInstance->registerObject("AIBridge");
   }
   return smInstance;
}

//-----------------------------------------------------------------------------

void AIBridgeManager::initPersistFields()
{
   addField("port", TypeS32, Offset(mPort, AIBridgeManager),
      "TCP port to listen on for AI commands (default 9090).");

   Parent::initPersistFields();
}

bool AIBridgeManager::onAdd()
{
   if (!Parent::onAdd())
      return false;

   smInstance = this;
   Con::printf("AIBridgeManager: Ready. Use aiBridgeStart(%d) to begin listening.", mPort);
   return true;
}

void AIBridgeManager::onRemove()
{
   stopListener();
   mAgents.clear();

   if (smInstance == this)
      smInstance = NULL;

   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// Listener
//-----------------------------------------------------------------------------

bool AIBridgeManager::startListener(U32 port)
{
   mPort = port;

   // Use TorqueScript's TCPObject for the listener.
   // This is wired up via console eval so it uses the existing TCP stack.
   const char *script = avar(
      "if (!isObject(AIBridgeTCP)) {"
      "   new TCPObject(AIBridgeTCP);"
      "}"
      "AIBridgeTCP.listen(%d);", port);

   Con::evaluatef("%s", script);

   mListening = true;
   Con::printf("AIBridgeManager: Listening on port %d", port);
   return true;
}

void AIBridgeManager::stopListener()
{
   if (mListening)
   {
      Con::evaluatef("if (isObject(AIBridgeTCP)) AIBridgeTCP.disconnect();");
      mListening = false;
      Con::printf("AIBridgeManager: Listener stopped.");
   }
}

//-----------------------------------------------------------------------------
// Per-frame processing
//-----------------------------------------------------------------------------

void AIBridgeManager::process()
{
   // The TCP callbacks happen through TorqueScript events.
   // This method can be used for periodic state broadcasts, etc.
}

//-----------------------------------------------------------------------------
// Agent management
//-----------------------------------------------------------------------------

void AIBridgeManager::registerAgent(U32 agentId, AIPlayer *player)
{
   // Check for existing.
   AgentEntry *existing = _findAgent(agentId);
   if (existing)
   {
      existing->player = player;
      return;
   }

   AgentEntry entry;
   entry.id = agentId;
   entry.player = player;
   entry.browser = NULL;
   mAgents.push_back(entry);

   Con::printf("AIBridgeManager: Agent %d registered.", agentId);
}

void AIBridgeManager::unregisterAgent(U32 agentId)
{
   for (U32 i = 0; i < mAgents.size(); i++)
   {
      if (mAgents[i].id == agentId)
      {
         mAgents.erase(i);
         Con::printf("AIBridgeManager: Agent %d unregistered.", agentId);
         return;
      }
   }
}

void AIBridgeManager::bindBrowser(U32 agentId, BrowserTexture *browser)
{
   AgentEntry *entry = _findAgent(agentId);
   if (entry)
   {
      entry->browser = browser;
      Con::printf("AIBridgeManager: Browser bound to agent %d.", agentId);
   }
}

AIBridgeManager::AgentEntry* AIBridgeManager::_findAgent(U32 id)
{
   for (U32 i = 0; i < mAgents.size(); i++)
   {
      if (mAgents[i].id == id)
         return &mAgents[i];
   }
   return NULL;
}

//-----------------------------------------------------------------------------
// Command processing
//-----------------------------------------------------------------------------

const char* AIBridgeManager::processCommand(const char *jsonCmd)
{
   // Minimal JSON parsing using Torque's built-in string tools.
   // For production, use RapidJSON (already in the engine).
   //
   // This is a simplified dispatcher. The real implementation
   // would use persistence/rapidjson for proper parsing.

   Con::printf("AIBridgeManager: Command received: %s", jsonCmd);

   // For now, route through TorqueScript where JSON parsing is easier.
   static char result[4096];
   dSprintf(result, sizeof(result),
      "{\"type\":\"result\",\"success\":true,\"message\":\"Command received\"}");
   return result;
}

//-----------------------------------------------------------------------------
// Command handlers
//-----------------------------------------------------------------------------

const char* AIBridgeManager::_handleMove(U32 id, F32 x, F32 y, F32 z)
{
   AgentEntry *entry = _findAgent(id);
   if (!entry || !entry->player)
      return "{\"type\":\"error\",\"message\":\"Agent not found\"}";

   entry->player->setMoveDestination(Point3F(x, y, z), true);

   static char buf[256];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"move\",\"id\":%d}", id);
   return buf;
}

const char* AIBridgeManager::_handleAim(U32 id, F32 x, F32 y, F32 z)
{
   AgentEntry *entry = _findAgent(id);
   if (!entry || !entry->player)
      return "{\"type\":\"error\",\"message\":\"Agent not found\"}";

   entry->player->setAimLocation(Point3F(x, y, z));

   static char buf[256];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"aim\",\"id\":%d}", id);
   return buf;
}

const char* AIBridgeManager::_handleAimObject(U32 id, U32 targetId)
{
   AgentEntry *entry = _findAgent(id);
   if (!entry || !entry->player)
      return "{\"type\":\"error\",\"message\":\"Agent not found\"}";

   SimObject *targetObj = Sim::findObject(targetId);
   GameBase *target = dynamic_cast<GameBase*>(targetObj);
   if (target)
      entry->player->setAimObject(target);

   static char buf[256];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"aimObject\",\"id\":%d}", id);
   return buf;
}

const char* AIBridgeManager::_handleFire(U32 id, U32 trigger, bool state)
{
   AgentEntry *entry = _findAgent(id);
   if (!entry || !entry->player)
      return "{\"type\":\"error\",\"message\":\"Agent not found\"}";

   entry->player->setMoveTrigger(trigger, state);

   static char buf[256];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"fire\",\"id\":%d}", id);
   return buf;
}

const char* AIBridgeManager::_handleBrowse(U32 id, const char *url)
{
   AgentEntry *entry = _findAgent(id);
   if (!entry)
      return "{\"type\":\"error\",\"message\":\"Agent not found\"}";
   if (!entry->browser)
      return "{\"type\":\"error\",\"message\":\"No browser bound to agent\"}";

   entry->browser->loadUrl(url);

   static char buf[512];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"browse\",\"id\":%d}", id);
   return buf;
}

const char* AIBridgeManager::_handleJavaScript(U32 id, const char *code)
{
   AgentEntry *entry = _findAgent(id);
   if (!entry)
      return "{\"type\":\"error\",\"message\":\"Agent not found\"}";
   if (!entry->browser)
      return "{\"type\":\"error\",\"message\":\"No browser bound to agent\"}";

   entry->browser->executeJavaScript(code);

   static char buf[256];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"js\",\"id\":%d}", id);
   return buf;
}

const char* AIBridgeManager::_handleSpawn(const char *datablock, F32 x, F32 y, F32 z)
{
   // Spawn via TorqueScript for simplicity.
   static char script[1024];
   dSprintf(script, sizeof(script),
      "%%ai = new AIPlayer() { dataBlock = \"%s\"; position = \"%g %g %g\"; };"
      "%%ai.registerObject(); return %%ai.getId();",
      datablock, x, y, z);

   Con::EvalResult evalResult = Con::evaluatef("%s", script);
   const char *result = evalResult.valid ? evalResult.value.getString() : "0";

   static char buf[256];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"spawn\",\"newId\":%s}", result);
   return buf;
}

const char* AIBridgeManager::_handleQuery(U32 id)
{
   return getAgentState(id);
}

const char* AIBridgeManager::_handleExec(const char *script)
{
   Con::EvalResult evalResult = Con::evaluatef("%s", script);
   const char *result = evalResult.valid ? evalResult.value.getString() : "";

   static char buf[4096];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"result\",\"success\":true,\"action\":\"exec\",\"output\":\"%s\"}", result);
   return buf;
}

//-----------------------------------------------------------------------------
// State queries
//-----------------------------------------------------------------------------

const char* AIBridgeManager::getAgentState(U32 agentId)
{
   AgentEntry *entry = _findAgent(agentId);
   if (!entry || !entry->player)
      return "{\"type\":\"error\",\"message\":\"Agent not found\"}";

   AIPlayer *ai = entry->player;
   Point3F pos = ai->getPosition();
   Point3F aim = ai->getAimLocation();
   Point3F dest = ai->getMoveDestination();
   F32 speed = ai->getMoveSpeed();

   static char buf[1024];
   dSprintf(buf, sizeof(buf),
      "{\"type\":\"state\",\"id\":%d,"
      "\"pos\":[%.2f,%.2f,%.2f],"
      "\"aim\":[%.2f,%.2f,%.2f],"
      "\"dest\":[%.2f,%.2f,%.2f],"
      "\"speed\":%.2f,"
      "\"hasBrowser\":%s}",
      agentId,
      pos.x, pos.y, pos.z,
      aim.x, aim.y, aim.z,
      dest.x, dest.y, dest.z,
      speed,
      entry->browser ? "true" : "false");
   return buf;
}

const char* AIBridgeManager::getAllAgentsState()
{
   static char buf[8192];
   dStrcpy(buf, "{\"type\":\"allAgents\",\"agents\":[", sizeof(buf));

   for (U32 i = 0; i < mAgents.size(); i++)
   {
      if (i > 0) dStrcat(buf, ",", sizeof(buf));
      const char *state = getAgentState(mAgents[i].id);
      dStrcat(buf, state, sizeof(buf));
   }

   dStrcat(buf, "]}", sizeof(buf));
   return buf;
}

//-----------------------------------------------------------------------------
// Console functions
//-----------------------------------------------------------------------------

DefineEngineFunction(aiBridgeStart, bool, (U32 port), (9090),
   "Start the AI bridge listener.\n"
   "@param port TCP port to listen on (default 9090).\n"
   "@return True if started successfully.\n")
{
   return AIBridgeManager::get()->startListener(port);
}

DefineEngineFunction(aiBridgeStop, void, (), ,
   "Stop the AI bridge listener.\n")
{
   AIBridgeManager::get()->stopListener();
}

DefineEngineFunction(aiBridgeRegisterAgent, void, (U32 agentId, U32 playerId), ,
   "Register an AIPlayer as an AI-controllable agent.\n"
   "@param agentId Unique agent identifier.\n"
   "@param playerId SimObject ID of the AIPlayer.\n")
{
   SimObject *obj = Sim::findObject(playerId);
   AIPlayer *player = dynamic_cast<AIPlayer*>(obj);
   if (player)
      AIBridgeManager::get()->registerAgent(agentId, player);
   else
      Con::errorf("aiBridgeRegisterAgent: Object %d is not an AIPlayer.", playerId);
}

DefineEngineFunction(aiBridgeUnregisterAgent, void, (U32 agentId), ,
   "Unregister an agent from the AI bridge.\n")
{
   AIBridgeManager::get()->unregisterAgent(agentId);
}

DefineEngineFunction(aiBridgeBindBrowser, void, (U32 agentId, U32 browserId), ,
   "Bind a BrowserTexture to an agent.\n"
   "@param agentId The agent to bind to.\n"
   "@param browserId SimObject ID of the BrowserTexture.\n")
{
   SimObject *obj = Sim::findObject(browserId);
   BrowserTexture *browser = dynamic_cast<BrowserTexture*>(obj);
   if (browser)
      AIBridgeManager::get()->bindBrowser(agentId, browser);
   else
      Con::errorf("aiBridgeBindBrowser: Object %d is not a BrowserTexture.", browserId);
}

DefineEngineFunction(aiBridgeCommand, const char *, (const char *json), ,
   "Send a JSON command to the AI bridge.\n"
   "@param json The JSON command string.\n"
   "@return JSON response string.\n")
{
   return AIBridgeManager::get()->processCommand(json);
}

DefineEngineFunction(aiBridgeGetState, const char *, (U32 agentId), ,
   "Get the world state for an agent as JSON.\n"
   "@param agentId The agent to query.\n"
   "@return JSON state string.\n")
{
   return AIBridgeManager::get()->getAgentState(agentId);
}

DefineEngineFunction(aiBridgeGetAllStates, const char *, (), ,
   "Get the world state for all agents as JSON.\n"
   "@return JSON string with all agent states.\n")
{
   return AIBridgeManager::get()->getAllAgentsState();
}
