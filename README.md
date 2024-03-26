# Ftp Client
[简体中文](README_CN.md)
## Introduction:

This is an FTP client written in Qt and C++. It utilizes the QFtp class for its functionality. The client is simple in nature and the following features are described below.

## Features:

FTP Server Address Input Box: The client has a text box for entering the FTP server address. The default username is "anonymous" for anonymous login. The username and password can be modified within the program. However, there is no text box provided in the program interface for user input. You can add it yourself as needed.

Connecting to Server Port: The default port for connecting to the server is 8021. There is a macro definition for the port in the program, which can be modified according to your needs.

File Batch Upload and Download: The client supports batch upload and download of files within the same folder.

Folder Operations: The client supports right-click creation of folders and batch deletion of files.

### Preview
<img src="./ftp-preview.png" width="60%" height="auto" style="display: block;">
