import asyncio
import websockets

from config import PORT, TIMEOUT_SECONDS
import video
import time

# Track all connections
CONNECTIONS = set()
last_conn = 0

# Video
vid = None

async def handler(websocket):
    """
    Connection handler, tracks all connections
    Removes connections on close
    """
    global vid, last_conn
    CONNECTIONS.add(websocket)
    async for message in websocket:
        try:
            if time.time() - last_conn > TIMEOUT_SECONDS:
                last_conn = time.time()

                print("Adding video", message)
                tmp = video.Video(message)
                if vid: vid.stream.stop()
                vid = tmp
        except Exception as e:
            print(e) # TODO: should send error to client somehow
    try:
        await websocket.wait_closed()
    finally:
        CONNECTIONS.remove(websocket)
        if not len(CONNECTIONS):
            print("No more users... terminating video")
            if vid: vid.stream.stop()
            vid = None

def message_all(message):
    """
    Send a message to all clients
    :param message: String or bytes like object
    """
    websockets.broadcast(CONNECTIONS, message)

async def main_loop():
    """
    Repeatedly streams a video to all
    current clients
    """
    print("Server started! Port: " + str(PORT))
    while True:
        # No video loaded, busy-wait
        if not vid:
            await asyncio.sleep(1)
            continue

        if vid.new_frame and vid.data is not None:
            message_all(bytes(vid.data))
        vid.new_frame = False

        await asyncio.sleep(1 / (vid.stream.framerate + 0.0001))
        vid.process()

async def main():
    async with websockets.serve(handler, "", PORT, compression=None, open_timeout=None):
        await main_loop()

if __name__ == "__main__":
    asyncio.run(main())
