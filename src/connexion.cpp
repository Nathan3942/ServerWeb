/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connexion.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 17:45:27 by njeanbou          #+#    #+#             */
/*   Updated: 2025/10/01 16:39:47 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/connexion.hpp"

Connexion::Connexion() : fd(-1), state(READING), bytes_sent(0)
{

}

Connexion::Connexion(int _fd) : fd(_fd), state(READING), bytes_sent(0), response(NULL)
{

}

Connexion::~Connexion()
{
	delete response;
}


State	Connexion::get_state() const
{
	return (state);
}

int	Connexion::get_fd() const
{
	return (fd);
}

std::vector<char>& Connexion::get_write_buffer()
{
	return write_buffer;
}

size_t& Connexion::get_bytes_sent()
{
	return bytes_sent;
}

Response*	Connexion::get_response()
{
	return (response);
}

void	Connexion::set_response(Response* res)
{
	if (response)
		delete response;
	response = res;
}

void	Connexion::set_bytes_sent(size_t _byte_sent)
{
	bytes_sent = _byte_sent;
}

void Connexion::set_write_buffer(const std::vector<char>& data)
{
	write_buffer = data;
	bytes_sent = 0;
}

void	Connexion::set_state(State _state)
{
	state = _state;
	std::cout << "[Connexion fd=" << fd << "] State changed to "
	          << (_state == READING ? "READING" : _state == WRITING ? "WRITING" : "CLOSED")
	          << std::endl;
}

void Connexion::clear()
{
	write_buffer.clear();
	bytes_sent = 0;
}

