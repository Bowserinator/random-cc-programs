# ComputerCraft Video Player

Play youtube videos (or any stream that yt-dlp supports I believe) in Minecraft! Does not support sound.

## Installation (Server)

In the server folder, modify config.py:

```python
PORT = 9123                   # Port for websocket server
SCREEN_RES = (39 * 2, 12 * 2) # Resolution of your computercraft monitor, in chars
TIMEOUT_SECONDS = 5           # Will ignore any video requests made before this timeout since the last one
```

Then to run:

```
pip3 install -r requirements.txt
python3 server.py
```

If you have issues with PIL like `Dither not found` etc... try the following to update to the latest version:

```
python3 -m pip install --upgrade Pillow
```

## Installation (Client)

In an internet enabled ComputerCraft computer (prefer advanced computer and monitor), download `stream.lua`.

Then edit the file `stream.lua` and change `WS_URL` to the url of your server. **(IMPORTANT)**

To run (note: if video is invalid or the rate limit is hit there **will not be an error it will just hang (I was lazy)**):

```
stream <video url>
```

For additional info run `stream --help`. Note that all clients share a video player (for performance reasons), so changing 
the current video will change the video for all clients.

## It's not working!
1. Make sure compression is disabled, some (older?) versions of CC or Lua may not support compression for websockets
2. Make sure `http` and `http_websockets` are enabled in your CC config (per world under `your save folder/serverconfig`)
3. Make sure your server is accessible (obviously)
