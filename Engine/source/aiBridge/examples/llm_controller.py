#!/usr/bin/env python3
"""
LLM Controller for Torque3D Virtual Office
-------------------------------------------
Connects to the AI Bridge TCP listener and uses an LLM (OpenAI/Claude)
to control AI agents in the 3D virtual office.

Usage:
    python llm_controller.py --port 9090 --api-key YOUR_KEY

The LLM receives world state and decides what agents should do:
- Navigate the office
- Browse websites on their screens
- Interact with objects
- Execute JavaScript on browser screens
"""

import json
import socket
import time
import argparse
from openai import OpenAI


SYSTEM_PROMPT = """You are an AI controller for a virtual 3D office.
You control multiple AI agents who work at desks with browser screens.

Available actions (respond with JSON array):
- {"action": "move", "id": AGENT_ID, "x": X, "y": Y, "z": Z} - Move agent
- {"action": "aim", "id": AGENT_ID, "x": X, "y": Y, "z": Z} - Look at point
- {"action": "browse", "id": AGENT_ID, "url": "URL"} - Load URL on agent's screen
- {"action": "js", "id": AGENT_ID, "code": "JS_CODE"} - Run JS on agent's screen
- {"action": "say", "id": AGENT_ID, "message": "TEXT"} - Agent speaks
- {"action": "query", "id": AGENT_ID} - Get agent's current state

Agents:
  1 = Research Agent (desk 1) - searches the web for information
  2 = Code Agent (desk 2) - writes and reviews code on GitHub
  3 = Comms Agent (desk 3) - handles emails and messages

Think about what each agent should be doing and issue commands accordingly.
Respond with a JSON array of actions to execute this tick.
"""


class TorqueBridge:
    """TCP connection to Torque3D AI Bridge."""

    def __init__(self, host="localhost", port=9090):
        self.host = host
        self.port = port
        self.sock = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        self.sock.settimeout(5.0)
        print(f"Connected to Torque3D at {self.host}:{self.port}")

    def send_command(self, cmd: dict) -> dict:
        """Send a JSON command and receive the response."""
        payload = json.dumps(cmd) + "\n"
        self.sock.sendall(payload.encode("utf-8"))
        data = self.sock.recv(8192).decode("utf-8")
        return json.loads(data) if data.strip() else {}

    def get_all_states(self) -> dict:
        """Query all agent states."""
        return self.send_command({"action": "queryAll"})

    def close(self):
        if self.sock:
            self.sock.close()


def run_loop(bridge: TorqueBridge, client: OpenAI, model: str = "gpt-4o-mini"):
    """Main AI control loop."""
    messages = [{"role": "system", "content": SYSTEM_PROMPT}]

    while True:
        # 1. Get world state
        try:
            world_state = bridge.get_all_states()
        except Exception as e:
            print(f"Error getting state: {e}")
            time.sleep(1)
            continue

        # 2. Ask LLM what to do
        messages.append({
            "role": "user",
            "content": f"Current world state:\n{json.dumps(world_state, indent=2)}\n\nWhat should the agents do next?"
        })

        response = client.chat.completions.create(
            model=model,
            messages=messages,
            temperature=0.7,
            max_tokens=1024,
        )

        reply = response.choices[0].message.content
        messages.append({"role": "assistant", "content": reply})

        # Keep conversation history manageable
        if len(messages) > 20:
            messages = [messages[0]] + messages[-10:]

        # 3. Parse and execute actions
        try:
            actions = json.loads(reply)
            if not isinstance(actions, list):
                actions = [actions]

            for action in actions:
                print(f"  -> {action}")
                result = bridge.send_command(action)
                print(f"  <- {result}")

        except json.JSONDecodeError:
            print(f"LLM returned non-JSON: {reply[:200]}")

        # 4. Tick rate (don't spam the LLM)
        time.sleep(2.0)


def main():
    parser = argparse.ArgumentParser(description="LLM Controller for Torque3D")
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=9090)
    parser.add_argument("--api-key", required=True)
    parser.add_argument("--model", default="gpt-4o-mini")
    args = parser.parse_args()

    bridge = TorqueBridge(args.host, args.port)
    bridge.connect()

    client = OpenAI(api_key=args.api_key)

    try:
        run_loop(bridge, client, args.model)
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        bridge.close()


if __name__ == "__main__":
    main()
