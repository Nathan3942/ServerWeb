/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connexion.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 17:42:12 by njeanbou          #+#    #+#             */
/*   Updated: 2025/10/01 16:33:24 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNEXION_HPP
#define CONNEXION_HPP

#include <string>
#include <iostream>
#include <vector>

#include "response.hpp"

enum State {READING, WRITING, CLOSED};

class Response;

class Connexion
{
    private:
        int fd;
        State state;
        std::vector<char> write_buffer;
        size_t  bytes_sent;
        Response* response;
    
    public:
		Connexion();
        Connexion(int _fd);
        ~Connexion();

        State get_state() const;
		int	get_fd() const;
        std::vector<char>& get_write_buffer();
        size_t& get_bytes_sent();
		Response*	get_response();

        void	set_response(Response* res);
        void	set_state(State _state);
        void    set_write_buffer(const std::vector<char>& data);
		void	set_bytes_sent(size_t _byte_sent);

        void	clear();
};


#endif
