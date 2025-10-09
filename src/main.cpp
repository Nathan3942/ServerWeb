/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:58:45 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/09 16:33:26 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

static	Server* global_server = NULL;

void	signal_handler(int signal)
{
	if (signal == SIGINT && global_server)
	{
		global_server->shutdown();
	}
}

int main(int ac, char **av)
{
    std::string conf;
    if (ac == 2)
        conf = av[1];
    else
        conf = "config.conf";
    // Ignore le signal SIGPIPE pour éviter que send sur socket fermée plante le process
    signal(SIGPIPE, SIG_IGN);
    try
    {
        Server* s = new Server(conf.c_str());

        global_server = s;

        struct sigaction sa;
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);

        s->start();
        
        delete s;
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }
    return (0);
}

//ab -n 100 -c 20 http://127.0.0.1:8080/ test stress

//strace -p <PID_DU_SERVEUR>

//sudo lsof -i :8080

/*curl -X PATCH http://localhost:port        # → 405
curl -X POST http://localhost:port         # → 411 si pas de Content-Length
curl -X POST -d @bigfile.txt http://...    # → 413
curl -X FOO http://localhost:port          # → 400*/
