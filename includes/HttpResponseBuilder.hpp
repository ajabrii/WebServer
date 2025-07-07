/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseBuilder.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:53:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 11:53:06 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

#include <string>
#include <map>
#include "HttpResponse.hpp"

class HttpResponseBuilder {
private:
    HttpResponse response;

public:
    HttpResponseBuilder& setVersion(const std::string& version);
    HttpResponseBuilder& setStatus(int code, const std::string& text);
    HttpResponseBuilder& setHeader(const std::string& key, const std::string& value);
    HttpResponseBuilder& setBody(const std::string& body);

    HttpResponse build() const;
};