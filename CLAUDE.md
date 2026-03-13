This is a project for a game simulator with two parts:

1. A C++ backend web service written with the Drogon http framework in the `srcBackend` directory
2. A JavaScript frontend application built with vite in the `srcFrontend` directory to display the UI in a browser

It builds a container image which has both frontend and backend parts in the same image.
