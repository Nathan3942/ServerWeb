/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:18:38 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/20 18:20:24 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

Server::Server(const char* _conf) : isRunning(false)
{
	conf = new Config(_conf);
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		std::cerr << "epoll_creat1: can't creat\n";
		delete conf;
		shutdown();
	}

	for (size_t i = 0; i < conf->get_port().size(); ++i)
	{
		std::cout << "Port : " << conf->get_port()[i] << std::endl;
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0)
		{
			std::cerr << "socket: can't set socket\n";
			delete conf;
			shutdown();
		}

		int opt = 1;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			std::cerr << "set socket: can't set\n";
			return ;
		}

		if (set_nonblocking(server_fd) == -1)
		{
			throw std::runtime_error("set_nonblocking: can't set\n");
		}

		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;

		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(conf->get_port()[i]);

		if (bind(server_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			std::cerr << "Bind: Permission denied\n";
			delete conf;
			shutdown();
		}

		if (listen(server_fd, 5) < 0)
		{
			std::cerr << "listen: can't listen fd\n";
			delete conf;
			shutdown();
		}

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = server_fd;

		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
		{
			std::cerr << "epoll_ctl: listen socket\n";
			delete conf;
			shutdown();
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
	const int EPOLL_TIMOUT_MS = 10000;
	struct epoll_event events[max_events];
	isRunning = true;

	while (isRunning)
	{
		// std::cout << "Debut de boucle\nEvent : ";
		int nfds = epoll_wait(epoll_fd, events, max_events, EPOLL_TIMOUT_MS);
		if (nfds == -1)
		{
			if (errno == EINTR)
        		continue;
			std::cerr << "epoll_wait: Error\n";
			continue;
		}

		if (nfds == 0)
		{
			for (std::map<int, Connexion*>::iterator it = clients.begin(); it != clients.end(); )
			{
				Connexion* conn = it->second;
				std::cout << "Closing connection: fd=" << conn->get_fd() << " for timeout" << std::endl;
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->get_fd(), NULL);
				close(conn->get_fd());
				delete conn;
				clients.erase(it++);
			}
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
				std::cout << "Closing connection: fd=" << fd << std::endl;
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
				close(fd);
				delete conn;
				clients.erase(fd);
			}
		}
	}
}


Config* Server::get_conf() const
{
	return (conf);
}
