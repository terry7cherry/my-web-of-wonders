#pragma once
/**
 * @file API.h
 * @brief Backend API server entry points, compiled only for FRONTEND builds.
 */

namespace API
{
#ifdef FRONTEND
    /**
     * @brief Start the HTTP API server for the frontend to communicate with.
     */
    void startAPIServer();
#endif
}
