/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:18:38 by ichpakov          #+#    #+#             */
/*   Updated: 2025/07/24 19:38:25 by njeanbou         ###   ########.fr       */
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
		perror("epoll_creat1");
		exit(1);
	}

	for (size_t i = 0; i < conf->get_port().size(); ++i)
	{
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0) {
			perror("socket");
			exit(1);
		}

		int opt = 1;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			perror("Error set socket");
			return ;
		}

		if (set_nonblocking(server_fd) == -1)
		{
			perror("set_nonblocking");
			exit(1);
		}

		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;

		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(conf->get_port()[i]);

		if (bind(server_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			perror("bind");
			exit(1);
		}

		if (listen(server_fd, 5) < 0) {
			perror("listen");
			exit(1);
		}

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = server_fd;

		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
		{
			perror("epoll_ctl: listen socket");
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


void	Server::shutdown()
{
	std::cout << "\nShutting down server..." << std::endl;

	for (std::map<int, Connexion>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		int fd = it->first;
		if (it->second.get_response())
		{
			it->second.get_response()->close();
			delete it->second.get_response();
		}
		close(fd);
	}
	clients.clear();

	close_socket();

	if (epoll_fd != -1)
		close(epoll_fd);
	isRunning = false;
}

void	Server::change_host()
{
	std::string host_name = "kaka.com";
	std::ofstream host_file("/etc/hosts", std::ios::app);
	std::ifstream host_file2("/etc/hosts");
	if (!host_file)
	{
		std::cerr << "Impossible d'ouvrir /etc/hosts, permission insuffisante!\n";
		return ;
	}
	std::string line;
	while (getline(host_file2, line))
	{
		if (line.find(host_name) != std::string::npos)
		{
			std::cerr << "Redirection deja effectue!\n";
			return ;
		}
	}
	host_file << "127.0.0.1 " << host_name << "\n";
	host_file.close();
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
		perror("accept");
		return ;
	}

	set_nonblocking(client_fd);

	clients[client_fd] = Connexion(client_fd);

	struct epoll_event	ev;
	ev.events = EPOLLIN;
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
	{
		perror("epoll_ctl: client");
		close(client_fd);
	}
}

// 

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
			perror("epoll_wait");
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

			std::map<int, Connexion>::iterator it = clients.find(fd);
			if (it == clients.end())
				continue;
			Connexion &conn = it->second;

			// Lecture de la requête
			if (events[i].events & EPOLLIN)
			{
				std::cout << "Nouvelle requete:" << std::endl;
				Request req(fd, conf->get_index()[0]);

				if (req.get_raw_request().empty())
					conn.set_state(CLOSED);
				else if (req.get_raw_request().find("\r\n\r\n") != std::string::npos)
				{
					std::cout << "fin de fichier" << std::endl;
					Response* res = new Response(req.get_path(), req, conf->get_root(), conf->get_error());
					conn.set_response(res);
					conn.set_write_buffer(res->get_next_chunk());
					conn.set_state(WRITING);

					// Modifier les événements pour écouter EPOLLOUT
					struct epoll_event ev;
					ev.events = EPOLLOUT;
					ev.data.fd = fd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
					{
						perror("epoll_ctl: mod EPOLLOUT");
						conn.set_state(CLOSED);
					}
				}
			}

			//Envoi de la réponse
			if (events[i].events & EPOLLOUT)
			{
				if (conn.get_state() == WRITING)
				{
					std::vector<char>& buf = conn.get_write_buffer();
					size_t total = buf.size();
					ssize_t sent = send(fd, &buf[conn.get_bytes_sent()], total - conn.get_bytes_sent(), 0);
					//std::cout << &buf[conn.get_bytes_sent()] << std::endl;
					if (sent < 0)
					{
						perror("send");
						conn.set_state(CLOSED);
						continue;
					}
					conn.set_bytes_sent(conn.get_bytes_sent() + sent);

					if (conn.get_bytes_sent() == buf.size())
					{
						conn.clear();
						
						Response* res = conn.get_response();
						if (res)
						{
							std::vector<char> chunk = res->get_next_chunk();
							if (!chunk.empty())
							{
								conn.get_write_buffer() = chunk;
								continue;
							}
							else
							{
								res->close();
								delete res;
								conn.set_response(NULL);
								conn.set_state(CLOSED);
								break;
							}
						}
					}
					
					struct epoll_event ev;
					ev.events = EPOLLIN;
					ev.data.fd = fd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
					{
						perror("epoll_ctl: restore EPOLLIN");
						conn.set_state(CLOSED);
					}
					else
						conn.set_state(READING);
				}
			}

			// close les fd
			if (conn.get_state() == CLOSED)
			{
				if (conn.get_response())
				{
					conn.get_response()->close();
					delete conn.get_response();
				}
				std::cout << "Closing connection: fd=" << fd << std::endl;
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
				close(fd);
				clients.erase(fd);
			}
			
			// std::cout << "Fin de boucle\n";
		}
	}
}

// ssize_t Server::send_all(Connexion &conn, const char* buf, size_t len)
// {  
// 	size_t total_sent = 0;
//     struct epoll_event ev;
// 	ev.events = EPOLLOUT;
// 	ev.data.fd = conn.get_fd();

// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn.get_fd(), &ev) == -1)
// 	{
// 		perror("epoll_ctl: mod EPOLLOUT");
// 		conn.set_state(CLOSED);
// 		return (-1);
// 	}

// 	while (total_sent < len)
// 	{
// 		struct epoll_event out_event;
// 		int ready = epoll_wait(epoll_fd, &out_event, 1, -1);

// 		if (ready < 0)
// 		{
// 			perror("epoll_wait: EPOLLOUT");

// 			conn.set_state(CLOSED);
// 			return (-1);
// 		}
// 		else if (ready == 0)
// 		{
// 			//std::cerr << "Timout waiting for socket be writable\n";
// 			usleep(1000);
// 			continue ;
// 		}
// 		std::cout.write(buf + total_sent, len - total_sent);
// 		if (out_event.events & EPOLLOUT)
// 		{
// 			ssize_t sent = send(ev.data.fd, buf + total_sent, len - total_sent, 0);
// 			if (sent <= 0)
// 			{
// 				std::cerr << "Failed to send data\n";
// 				conn.set_state(CLOSED);
// 				return (-1);
// 			}
// 			total_sent += sent;
// 		}
// 	}

// 	ev.events = EPOLLIN;
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn.get_fd(), &ev) == -1)
// 	{
// 		perror("epoll_ctl: restore EPOLLIN");
// 		conn.set_state(CLOSED);
// 		return (-1);
// 	}
// 	conn.set_state(READING);
// 	return (total_sent);
// }



// void Server::start()
// {
// 	const int max_events = 100;
// 	struct epoll_event events[max_events];
// 	isRunning = true;

// 	while (isRunning)
// 	{
// 		int nfds = epoll_wait(epoll_fd, events, max_events, -1);
// 		if (nfds == -1)
// 		{
// 			perror("epoll_wait");
// 			continue;
// 		}

// 		for (int i = 0; i < nfds; ++i)
// 		{
// 			int fd = events[i].data.fd;
			
// 			if (is_listen_socket(fd))
// 			{
// 				accept_connection(fd);
// 			}
// 			else if (events[i].events & EPOLLIN)
// 			{
// 				std::map<int, Connexion>::iterator it = clients.find(fd);
// 				if (it == clients.end())
// 					continue;
// 				Connexion &conn = it->second;
// 				Request req(fd);

// 				if (req.get_raw_request() == "")
// 				{
// 				 	conn.set_state(CLOSED);
// 				}

// 				if (req.get_raw_request().find("\r\n\r\n") != std::string::npos)
// 				{
// 					// Request req(fd); // traite la requête (parse)
// 					Responce res(req.get_path());
// 					std::vector<char> response = res.get_response();
// 					struct epoll_event ev;
// 					conn.set_state(WRITING);
// 					ev.events = EPOLLOUT | EPOLLET;
// 					ev.data.fd = fd;
// 					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
// 					if (events[i].events & EPOLLOUT)
// 						send_all(conn, &response[0], response.size());
// 				}

// 				if (conn.get_state() == CLOSED)
// 				{
// 					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
// 					close(events[i].data.fd);
// 					clients.erase(fd);
// 				}

// 			}
// 		}
// 	}
// }





// ssize_t Server::send_all(Connexion &conn)
// {  
// 	size_t total_sent = 0;
// 	size_t len = conn.get_write_buffer().size();
//     struct epoll_event ev;
// 	ev.events = EPOLLOUT;
// 	ev.data.fd = conn.get_fd();

// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn.get_fd(), &ev) == -1)
// 	{
// 		perror("epoll_ctl: mod EPOLLOUT");
// 		conn.set_state(CLOSED);
// 		return (-1);
// 	}

// 	while (total_sent < len)
// 	{
// 		struct epoll_event out_event;
// 		int ready = epoll_wait(epoll_fd, &out_event, 1, 500);

// 		if (ready < 0)
// 		{
// 			perror("epoll_wait: EPOLLOUT");
// 			conn.set_state(CLOSED);
// 			return (-1);
// 		}
// 		else if (ready == 0)
// 		{
// 			usleep(1000);
// 			continue ;
// 		}
// 		std::cout.write(conn.get_write_buffer().data() + total_sent, len - total_sent);
// 		if (out_event.events & EPOLLOUT)
// 		{
// 			ssize_t sent = send(conn.get_fd(), conn.get_write_buffer().data() + total_sent, len - total_sent, 0);
// 			if (sent <= 0)
// 			{
// 				std::cerr << "Failed to send data\n";
// 				conn.set_state(CLOSED);
// 				return (-1);
// 			}
// 			total_sent += sent;
// 		}
// 	}

// 	ev.events = EPOLLIN | EPOLLET;
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn.get_fd(), &ev) == -1)
// 	{
// 		perror("epoll_ctl: restore EPOLLIN");
// 		conn.set_state(CLOSED);
// 		return (-1);
// 	}
// 	conn.set_state(READING);
// 	return (total_sent);
// }




// void Server::start()
// {
// 	const int	max_events = 100;
// 	struct epoll_event events[max_events];
	
//     isRunning = true;
//     while (isRunning)
// 	{
// 		int	nfds = epoll_wait(epoll_fd, events, max_events, -1);
// 		if (nfds == -1)
// 		{
// 			perror("epoll_wait");
// 			continue;
// 		}

// 		for (int i = 0; i < nfds; ++i)
// 		{
// 			int fd = events[i].data.fd;

// 			if (is_listen_socket(fd))
// 				accept_connection(fd);
// 			else
// 			{
// 				std::map<int, Connexion>::iterator it = clients.find(fd);
// 				if (it == clients.end())
// 					continue;

// 				Connexion &conn = it->second;

// 				if (events[i].events & EPOLLIN)
// 				{
// 					// char buffer[1024];
// 					// ssize_t	bytes = recv(fd, buffer, sizeof(buffer), 0);
// 					// if (bytes <= 0)
// 					// {
// 					// 	conn.set_state(CLOSED);
// 					// }
// 					// else
// 					// {
// 						// conn.get_read_buffer().append(buffer, bytes);
// 						// if (conn.get_read_buffer().find("\r\n\r\n") != std::string::npos)
// 						// {
// 							Request req(fd);
// 							Responce res(req.get_path());
// 							conn.set_write_buffer(res.get_response());
// 							conn.set_state(WRITING);

// 							struct epoll_event ev;
// 							ev.events = EPOLLOUT | EPOLLET;
// 							ev.data.fd = fd;
// 							epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
// 						// }
// 					// }
// 				}
// 				else if (events[i].events & EPOLLOUT)
// 				{
// 					send_all(fd, conn.get_write_buffer().data(), conn.get_write_buffer().size());
// 				}

// 				if (conn.get_state() == CLOSED)
// 				{
// 					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
// 					close(events[i].data.fd);
// 					clients.erase(fd);
// 				}
// 				// Request req(fd);
// 				// Responce res(req.get_path());
				
// 				// const std::vector<char>& buffer = res.get_response();
// 				// // std::cout.write(&buffer[0], buffer.size());
// 				// std::cout << std::endl;
				
				
// 				// usleep(1000);
				
				
// 			}
// 		}
// 	}
// }

