/*
* NetAnalyzer -multithreaded portable statefull Network passive analyzer
* Project page: http://code.google.com/p/netanalyzer/
* Author: Elie Bursztein LSV, ENS-Cachan, CNRS, INRIA
* Email: elie@bursztein.eu
* Licence: GPL v2 
*
* netAnalyzer is (C) Copyright 2008  by Elie Bursztein 
*/
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include "../headers/structure.h"
#include "../headers/constant.h"
#include "../headers/function.h"
#include "../headers/protocol.h"

/*!
 *       \file parser_main.c
 *       \brief Used to parse configuration files.
 *
 *       Use a pseudo xml as apache do.
 *       \author  Elie
 *       \version 1.0
 *       \date    July 2006
 */


extern	t_option	option;
	_u32		parser_recurse;
	char		parser_current_file[1024];
/*!
 * Parser error handler 
 * @param file_name  the file parsed
 * @param line	the line of the error
 * \version 1.0
 * \date    2006
 * \author Elie
 */

void parse_error(char *file_name, int line)
{
	die("syntax error in file \"%s\" at line %d\n", file_name, line);
}

/*!
 * Verify if a  char is an affection one

	Used to provide a more flexible syntax such as space : = etc
 * @param c  the character to verify
 * @return true if it is an affectator false otherwise
 * \version 1.0
 * \date    2006
 * \author Elie
 */
 
_u8 parse_is_option_affect_op(char *c)
{
	if ( c[0] == ':' || c[0] == '=' || c[0] == ' ')
		return 1;
	else 
		return 0;
}


/*!
 * Verify if a  char is a comment starter

	Used to provide a more flexible syntax and use multiple style
 * @param c  the character to verify
 * @return true if it is an affectator false otherwise
 * \version 1.0
 * \date    2006
 * \author Elie
 */
 
_u8 parse_is_comment(char *c)
{
	if (isspace(c[0]) || c[0] == '#' || c[0] == '/' || c[0] == ';')
		return 1;
	else 
		return 0;
}

/*!
 * Parser entry point 
 * @param file_name  the file to parse
 * @see init()
 * @return if the parsing is successful
 * \version 1.0
 * \date    2006
 * \author Elie
 */

int parse_conf(char *file_name)
{
  FILE         *fp;
  char 		line[1024],*optvalue;
  ssize_t      r;
  int	       l = 0,i;
  
  bzero(parser_current_file, 1024);
	strncpy(parser_current_file, option.conf_path, 1023);
  strncat(parser_current_file, file_name, 1023 - strlen(parser_current_file));
	//printf("parsing : %s\n", parser_current_file);
  if (parser_recurse++ == 100)
	  die("Infinite Recursive file include last file in recursion is %s\n", parser_current_file);
  
  //try to read the file
  if (!(fp = fopen(parser_current_file, "r")))
	{
		die("Error openning the file %s\n", parser_current_file);
		return -1;

	}
	l = 0;
	/* Parsing the configuration files and the if the syntax is correct */
	while ((fgets(line, sizeof(line), fp)))
	{
		l++;
		//comment
		if (parse_is_comment(line))
			continue;
		//recursive inclusion
		optvalue = line;
		if (strncmp(line, "include", 7) == 0) {
			optvalue = optvalue + 7;
			if (parse_is_option_affect_op(optvalue))
				optvalue++;
			else
				die("%snetanalyzer.cfg:%d:\tParsing syntax error:file include syntax error\n", option.conf_path, l);
			i = strlen(optvalue);
			optvalue[i-1] = '\0';
			parse_conf(optvalue);
		} else if (!(r = strncmp(line,"<trafficPattern>",10))) 	//finding the section
		{		if(option.debug == 42)
					printf("Traffic pattern block parsing start in file %s at line %d\n", file_name, l);
				parse_traffic_pattern(fp, l);
		} else if (!(r = strncmp(line,"<General>",9)))
		{		if(option.debug == 42)
					printf("Main block parsing start in file %s at line %d\n", file_name, l);
			parse_main_block(fp, l);
		}  else if (!(r = strncmp(line,"<Services>",10)))
		{		if(option.debug == 42)
			printf("Services parsing start in file %s at line %d\n", file_name, l);
			parse_services_block(fp, l);
		} else if (!(r = strncmp(line,"<Tuning>",8)))
		{		if(option.debug == 42)
			printf("Tuning options block parsing start in file %s at line %d\n", file_name, l);
			parse_tuning_block(fp, l);
		} else if (!(r = strncmp(line,"<Macprefix>",11)))
		{		if(option.debug == 42)
			printf("Mac address parsing start in file %s at line %d\n", file_name, l);
			parse_macprefix_block(fp, l);
		} else {
			printf("%s", line);
			parse_error(file_name, l);
		}
		bzero(line,sizeof(line));
	}
  
  //if (line)
   // free(line);
  fclose(fp);
  
  return 1;
}
