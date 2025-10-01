/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:18:38 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/01 17:21:15 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

Server::Server(const char* _conf) : isRunning(false)
{
	//change_host();
	std::cout << "Debut config" << std::endl;
	conf = new Config(_conf);
	
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		std::cerr << "epoll_creat1: can't creat\n";
		exit(1);
	}

	for (size_t i = 0; i < conf->get_port().size(); ++i)
	{
		std::cout << "Port : " << conf->get_port()[i] << std::endl;
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0)
		{
			std::cerr << "socket: can't set socket\n";
			exit(1);
		}

		int opt = 1;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			std::cerr << "set socket: can't set\n";
			return ;
		}

		if (set_nonblocking(server_fd) == -1)
		{
			std::cerr << "set_nonblocking: can't set\n";
			exit(1);
		}

		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;

		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(conf->get_port()[i]);

		if (bind(server_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			std::cerr << "bind: Permission denied\n";
			exit(1);
		}

		if (listen(server_fd, 5) < 0)
		{
			std::cerr << "listen: can't listen fd\n";
			exit(1);
		}

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = server_fd;

		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
		{
			std::cerr << "epoll_ctl: listen socket\n";
			exit(1);
		}

		sockets.push_back(server_fd);
        std::cout << "Listening on port " << conf->get_port()[i] << std::endl;
	}
}


Server::~Server()
{
	delete conf;
}


void	Server::close_socket()
{
	for (size_t i = 0; i < sockets.size(); ++i)
		close(sockets[i]);
	sockets.clear();
}


void Server::shutdown()
{
    std::cout << "\nShutting down server..." << std::endl;

    for (std::map<int, Connexion*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        int fd = it->first;
        delete it->second;
        close(fd);
    }
    clients.clear();
    close_socket();

    if (epoll_fd != -1)
        close(epoll_fd);

    isRunning = false;
}


int	Server::set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return (-1);
	std::cout << "[FD " << fd << "] est maintenant non bloquant." << std::endl;
	return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}


bool	Server::is_listen_socket(int fd) const
{
	for (size_t i = 0; i < sockets.size(); ++i)
	{
		if (sockets[i] == fd)
			return (true);
	}
	return (false);
}


void	Server::accept_connection(int listen_fd)
{
	struct sockaddr_in client_addr;
	socklen_t	client_len = sizeof(client_addr);
	int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
	{
		std::cerr << "accept: connextion aren't accepted\n";
		return ;
	}

	set_nonblocking(client_fd);

	Connexion* new_conn = new Connexion(client_fd);
	clients[client_fd] = new_conn;

	struct epoll_event	ev;
	ev.events = EPOLLIN;
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl: client\n";
		close(client_fd);
	}
}


void Server::start()
{
	const int max_events = 100;
	struct epoll_event events[max_events];
	isRunning = true;

	while (isRunning)
	{
		// std::cout << "Debut de boucle\nEvent : ";
		int nfds = epoll_wait(epoll_fd, events, max_events, -1);
		if (nfds == -1)
		{
			//!\\/
			if (errno == EINTR)
        		continue;
			std::cerr << "epoll_wait: Error\n";
			continue;
		}

		for (int i = 0; i < nfds; ++i)
		{
			int fd = events[i].data.fd;

			if (is_listen_socket(fd))
			{
				accept_connection(fd);
				continue;
			}

			std::map<int, Connexion*>::iterator it = clients.find(fd);
			if (it == clients.end())
				continue;
			Connexion *conn = it->second;

			// Lecture de la requête
			if (events[i].events & EPOLLIN)
			{
				std::cout << "Nouvelle requete:" << std::endl;
				Request req(fd, *conf);

				if (req.get_raw_request().empty())
				{
					std::cout << "La requete est vide, passe la state en closed\n";
					conn->set_state(CLOSED);
				}
				else if (req.get_raw_request().find("\r\n\r\n") != std::string::npos)
				{
					std::cout << "fin de fichier" << std::endl;
					Response* res = new Response(req);
					conn->set_response(res);
					conn->set_write_buffer(res->get_next_chunk());
					conn->set_state(WRITING);

					// Modifier les événements pour écouter EPOLLOUT
					struct epoll_event ev;
					ev.events = EPOLLOUT;
					ev.data.fd = fd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
					{
						std::cerr << "epoll_ctl: mod EPOLLOUT\n";
						conn->set_state(CLOSED);
					}
				}
			}

			//Envoi de la réponse
			if (events[i].events & EPOLLOUT)
			{
				if (conn->get_state() == WRITING)
				{
					std::vector<char>& buf = conn->get_write_buffer();
					size_t total = buf.size();
					ssize_t sent = send(fd, &buf[conn->get_bytes_sent()], total - conn->get_bytes_sent(), 0);
					//std::cout << &buf[conn.get_bytes_sent()] << std::endl;
					if (sent < 0)
					{
						std::cerr << "send error\n";
						conn->set_state(CLOSED);
						continue;
					}
					conn->set_bytes_sent(conn->get_bytes_sent() + sent);

					if (conn->get_bytes_sent() == buf.size())
					{
						conn->clear();
						
						Response* res = conn->get_response();
						if (res)
						{
							std::vector<char> chunk = res->get_next_chunk();
							if (!chunk.empty())
							{
								conn->get_write_buffer() = chunk;
								continue;
							}
							else
							{
								res->close();
								// delete res;
								// conn->set_response(NULL);
								conn->set_state(CLOSED);
								break;
							}
						}
					}
					
					if (conn->get_state() != CLOSED)
					{
						struct epoll_event ev;
						ev.events = EPOLLIN;
						ev.data.fd = fd;
						if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
						{
							std::cerr << "epoll_ctl: restore EPOLLIN\n";
							conn->set_state(CLOSED);
						}
						else
							conn->set_state(READING);
					}
				}
			}

			// close les fd
			if (conn->get_state() == CLOSED)
			{
				// if (conn->get_response())
				// {
				// 	conn->get_response()->close();
				// 	// delete conn->get_response();
				// }
				std::cout << "Closing connection: fd=" << fd << std::endl;
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
				close(fd);
				delete conn;
				clients.erase(fd);
			}
			
			// std::cout << "Fin de boucle\n";
		}
	}
}


// void	Server::change_host()
// {
// 	std::string host_name = "kaka.com";
// 	std::ofstream host_file("/etc/hosts", std::ios::app);
// 	std::ifstream host_file2("/etc/hosts");
// 	if (!host_file)
// 	{
// 		std::cerr << "Impossible d'ouvrir /etc/hosts, permission insuffisante!\n";
// 		return ;
// 	}
// 	std::string line;
// 	while (getline(host_file2, line))
// 	{
// 		if (line.find(host_name) != std::string::npos)
// 		{
// 			std::cerr << "Redirection deja effectue!\n";
// 			return ;
// 		}
// 	}
// 	host_file << "127.0.0.1 " << host_name << "\n";
// 	host_file.close();
// }
