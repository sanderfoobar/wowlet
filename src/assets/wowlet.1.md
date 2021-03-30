% WOWLET(1) wowlet 1.0
% WOWlet
% March 2021

# NAME

wowlet — Lightweight Wownero Wallet  

# SYNOPSIS

**wowlet** [*OPTION*]  

# DESCRIPTION

**WOWlet** is a free, open-source Wownero desktop wallet for Linux with ports for Mac OS and Windows.  

# Options

**-h**, **\--help**

:   Displays usage information.

**-v**, **\--version**

:   Displays the current version number.

**\--use-local-tor**

:   Use system wide installed Tor instead of the bundled.

**\--tor-port**

:   Port of running Tor instance.

**\--debug**

:   Run program in debug mode.

**\--quiet**

:   Limit console output.

**\--use-local-tor**

:   Use system wide installed Tor instead of the bundled.

**\--stagenet**

:   Stagenet is for development purposes only.

**\--testnet**

:   Testnet is for development purposes only.

**\--wallet-file**

:   Path to wallet keys file.

**\--password**

:   Wallet password (escape/quote as needed).

**\--daemon-address**

:   Daemon address (IPv4:port).

**\--export-contacts**

:   Output wallet contacts as CSV to specified path.

**\--export-txhistory**

:   Output wallet transaction history as CSV to specified path.

**\--daemon**

:   Start wowlet in the background and start a websocket server (IPv4:port).

**\--daemon-password**

:   Password for connecting to the wowlet websocket service.

# EXAMPLES

**wowlet -v | \--version**

:   Displays software version and then exits.

**wowlet -h | \--help**

:   Displays software usage information and then exits.

**wowlet \--daemon 127.0.0.1:1234 \--daemon-password "sekrit"**

:   Wallet starts in the background and exposes a websocket port that you can connect to using a websocket client.

# BUGS

See Git Issues: <https://git.wownero.com/wowlet/wowlet/issues>  

# COPYRIGHT

(c) 2020-2021 The Monero Project.  

License: BSD-3-clause. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:  

1.  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.  

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
