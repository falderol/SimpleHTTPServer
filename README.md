# Web Server


## Purpose

The purpose of this assignment is to give students practice with:

- application-layer socket programming
- reading a protocol specification
- implementation HTTP server-side protocol
- low-level programming 


## Requirements

Use the official Hypertext Transfer Protocol (HTTP/1.1) specification as a reference:

- [https://tools.ietf.org/html/rfc2616](https://tools.ietf.org/html/rfc2616)

Create a general-purpose web server with the following characteristics:

- Implements a subset of the HTTP/1.1 specification
	- Must handle the following HTTP request types: GET, HEAD
	- Responds with an error if the request is malformed
- Listens for connections on a socket that is bound to a network port number
	- Port number is specified as a command line argument (see Command Arguments section)
- Handles non-persistent connections (support for persistent connections is not required)
- Can handle multiple requests simultaneously (using fork to spawn multiple processes)
- Determine if requested file exists (see File Handling section):
	- If yes, transmit file with appropriate HTTP response headers
	- If no, transmit appropriate HTTP error code and message


### File Handling

This HTTP server is a general purpose web server. It does not support dynamic content generation. It simply transmits files that are requested (via GET) to the HTTP client. 

The DOCROOT is a special directory on the server that holds all the files and subdirectories that should be accessible via the HTTP protocol. DOCROOT is provided as a command line argument (see Command Arguments section). Following are some example HTTP-to-filesystem path translations using `/tmp/www/htdocs` as the DOCROOT:

- http://localhost:8080/index.html
	- /tmp/www/htdocs/index.html
- http://localhost:8080/images/foo.jpg
	- /tmp/www/htdocs/images/foo.jpg
- http://localhost:8080/foo/styles/my.css
	- /tmp/www/htdocs/foo/styles/my.css
- ...


### Command Arguments

The web server must be started from the command line with two command line arguments:

1. DOCROOT: server path to web server files
2. PORT: what network port to use
	- Note: Use a port number higher than 1024 unless you have administrative/root privileges. Conventionally, port 8080 is a typical unprivileged port number used for HTTP (it can be any port number).

```shell
% ./http-server /tmp/www/htdocs 8080
```


### Language and Building

* Programs must compile using the GCC 5.x C compiler (gcc)
* Programs must compile and run on Linux
	- Programs may optionally be written to run on other platforms
* Programs may use the following libraries:
	- "C standard library" a.k.a "libc"
	- Linux system libraries (libraries that are included with less-than and greater-than symbols, e.g. `include <foo.h>`)
* Students are not permitted to use any libraries that implement part or all of the HTTP protocol
* Program must be written in standard ANSI C (C11)
	- Warning: gcc, by default, will accept non-standard C code
	- Tip: standard C online documentation can be found at [http://en.cppreference.com/w/c](http://en.cppreference.com/w/c)


## Turn In

This assignment uses a progressive grading scheme. Students are required to push incremental work to a designated repository.

**Commit and push** your work for this assignment to the following Git repository and subdirectory:

- *Git Repository*: `https://repo.cse.taylor.edu/work/{username}`
- *Project Subdirectory*: `course/cos331/project/http`

To get full credit, students must commit and push work that satisfies the milestone requirements on a schedule provided by the instructor.


### Milestones

- Milestone 1:
	- Parses and stores command line arguments
	- Listens on port for connection
- Milestone 2:
	- Handles new connection
	- Parses and stores HTTP requests
	- Opens requested file for reading
- Milestone 3:
	- Sends valid HTTP response
- Milestone 4:
	- Handles multiple requests simultaneously (using multi-process approach)