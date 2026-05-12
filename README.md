# MPV Video Registrar
Used to keep personal track of what videos were watched using [mpv player](https://github.com/mpv-player/mpv), categorize them, keep personal comments for them. Works with local files, as well as with any URLs the mpv player can play. It successfully handles different types of the same urls. For instance:

- `https://m.youtube.com/watch?v=4kHl4FoK1Ys&pp=ygUeeW91J3JlIG15IGhlYXJ0LCB5b3VyZSBteSBzb3Vs`
- `https://youtu.be/4kHl4FoK1Ys%3Fsi%3DQnoeBIrlzPBLL_xG`
- `https://youtube.com/watch?v=4kHl4FoK1Ys`

Are all the same url, and will be evidented as a single video.

Uses `sqllite3` to store data.
Uses `yt-dlp` to fetch video metadata.
Uses [cJSON by davegamble](https://github.com/davegamble/cjson)

# Build
- Install `sqlite3`: sudo apt install libsqlite3-dev, and use `-lsqlite3` flag in `gcc` command.

# TODO
- [ ] create a `Makefile`
- [ ] show test examples
- [ ] create a manpage for the program
