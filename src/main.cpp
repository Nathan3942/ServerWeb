/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:58:45 by ichpakov          #+#    #+#             */
/*   Updated: 2025/07/10 15:14:56 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

int main()
{
    // Ignore le signal SIGPIPE pour éviter que send sur socket fermée plante le process
    signal(SIGPIPE, SIG_IGN);
    std::vector<int> ports;
    ports.push_back(8080);
    ports.push_back(8081);
    ports.push_back(8082);
    Server* s = new Server(ports);
    s->start();
    
    delete s;
    return 0;
}

//ab -n 100 -c 20 http://127.0.0.1:8080/ test stress

//strace -p <PID_DU_SERVEUR>

//sudo lsof -i :8080