<!-- ci comment -->
![](https://img.shields.io/badge/CI-passed-green)

You can find a link to the downloadable tarball below (for x86-64 architecture).

|            |                                               |
| ---------- | --------------------------------------------- |
| Commit     | {{ .commit }}                                 |
| Logs       | {{ .logs | mdlink "View Logs" }}              |
| Download   | {{ .download | mdlink "Download Tarball" }}   |
| Expiration | {{ .expire | date "02 Jan 2006 15:04 CEST" }} |

To install it, unzip and then execute `sudo tar -C / --no-same-owner -h -xzf linux-enable-ir-emitter*.tar.gz`. 

This comment is updated automatically.
