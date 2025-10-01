/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:17:59 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/01 16:27:02 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//CLASSE DE BASE DU SERVEUR

#pragma once

#include "request.hpp"
#include "response.hpp"
#include "connexion.hpp"
#include "Config.hpp"

#include <sys/stat.h>
#include <sstream>
#include <algorithm>
#include <dirent.h>
#include <sys/wait.h>
#include <limits>

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <vector>
#include <sys/epoll.h>
#include <fcntl.h>
#include <map>
#include <signal.h>
#include <csignal>

class Connexion;

class Config;

class Server
{
    private:
        std::vector<int>    ports;
        std::vector<int>    sockets;
        std::map<int, Connexion*> clients;
		Config*	conf;
        int epoll_fd;
        bool isRunning; 

    public:
        Server(const char* _conf);
        ~Server();

        void    start();
        void    change_host();
        void    close_socket();
        int     set_nonblocking(int fd);
        void    accept_connection(int listen_fd);
        bool    is_listen_socket(int fd) const;
        ssize_t send_all(Connexion &conn, const char* buf, size_t len);
        void    shutdown();
};
