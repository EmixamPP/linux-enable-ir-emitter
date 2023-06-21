<!-- build comment -->
![Badge](https://img.shields.io/badge/CI-passed-green)

You can find a link to the downloadable tarball below.

|            |                                               |
| ---------- | --------------------------------------------- |
| Commit     | {{ .commit }}                                 |
| Logs       | {{ .logs | mdlink "View Logs" }}              |
| Download   | {{ .download | mdlink "Download Tarball" }}   |
| Expiration | {{ .expire | date "02 Jan 2006 15:04 CEST" }} |

To install it, execute `sudo tar -C / --no-same-owner -h -xzf linux-enable-ir-emitter.tar.gz`. 

This comment is updated automatically.
