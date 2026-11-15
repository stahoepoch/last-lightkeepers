"""Call the already-running local Unreal MCP server when tools are not attached.

Usage: python Tools/unreal_mcp_local.py list
       python Tools/unreal_mcp_local.py call TOOL_NAME '{"key":"value"}'
The URL is loopback only. Calls are serialized by each process invocation.
"""

import asyncio
import base64
import json
import sys
from pathlib import Path

from mcp import ClientSession
from mcp.client.streamable_http import streamablehttp_client


async def main():
    async with streamablehttp_client("http://127.0.0.1:8000/mcp", timeout=20,
                                     terminate_on_close=False) as streams:
        async with ClientSession(streams[0], streams[1]) as session:
            await session.initialize()
            if len(sys.argv) == 2 and sys.argv[1] == "list":
                result = await session.list_tools()
                for tool in result.tools:
                    print(json.dumps({"name": tool.name,
                                      "description": tool.description,
                                      "inputSchema": tool.inputSchema}, default=str))
                return
            if len(sys.argv) == 4 and sys.argv[1] == "call":
                result = await session.call_tool(sys.argv[2], json.loads(sys.argv[3]))
                print(json.dumps(result.model_dump(exclude_none=True), ensure_ascii=True))
                if result.isError:
                    raise SystemExit(1)
                return
            if len(sys.argv) in (3, 4) and sys.argv[1] == "describe":
                result = await session.call_tool(
                    "describe_toolset", {"toolset_name": sys.argv[2]})
                payload = json.loads(result.content[0].text)
                for tool in payload.get("tools", []):
                    if len(sys.argv) == 3 or sys.argv[3].lower() in tool["name"].lower():
                        print(json.dumps(tool, ensure_ascii=True))
                return
            if len(sys.argv) == 3 and sys.argv[1] == "names":
                result = await session.call_tool(
                    "describe_toolset", {"toolset_name": sys.argv[2]})
                payload = json.loads(result.content[0].text)
                for tool in payload.get("tools", []):
                    print(tool["name"])
                return
            if len(sys.argv) in (3, 4) and sys.argv[1] == "capture":
                pose = json.loads(sys.argv[3]) if len(sys.argv) == 4 else {}
                result = await session.call_tool("call_tool", {
                    "toolset_name": "EditorToolset.EditorAppToolset",
                    "tool_name": "CaptureViewport",
                    "arguments": {"captureTransform": pose,
                                  "annotations": {"gridSpacing": 0,
                                                  "gridExtent": 0,
                                                  "gridHeight": 0,
                                                  "maxLabelDistance": 0,
                                                  "classFilter": {"refPath": "/Script/Engine.Actor"},
                                                  "maxLabels": 0},
                                  "bShowUI": False}})
                if result.isError:
                    raise RuntimeError(result.content[0].text)
                payload = json.loads(result.content[0].text)["returnValue"]
                image_data = payload["image"]["data"]
                path = Path(sys.argv[2])
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(base64.b64decode(image_data))
                print(json.dumps({"path": str(path),
                                  "cameraLocation": payload["cameraLocation"],
                                  "cameraRotation": payload["cameraRotation"],
                                  "bytes": path.stat().st_size}))
                return
            raise SystemExit("usage: list | call TOOL_NAME JSON_ARGUMENTS")


if __name__ == "__main__":
    asyncio.run(main())
