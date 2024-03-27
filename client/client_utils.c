#include "client.h"

int	countWords(const char *str)
{
	bool	inWord = false;
	int		count = 0;

	while (*str != '\0')
	{
		//current character is whitespace
		if (isspace(*str))
		{
			//previously inside a word, increment word count and reset flag
			if (inWord == true)
			{
				count++;
				inWord = false;
			}
		}
		else //current character not whitespace
			inWord = true;
		str++; // move to the next character
	}

	//count+1 if  last character not whitespace (to count last word)
	if (inWord == true)
		count++;
	return (count);
}



char* getName(const char *str)
{
	bool	inWord = false;
	char	*word = NULL;
	int		count = 0;
	int		start = -1;
	int		end = -1;

	for (int i = 0; str[i] != '\0'; i++)
	{
		if (isspace(str[i]))
		{
			if (inWord)
			{
				count++;
				inWord = false;
				if (count == 2)
				{
					start = i + 1; // set start index to  character after whitespace
					inWord = true;
				}
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				if (count == 1)//first word
					start = i;
			}
		}

		if (count == 2 && !inWord) { // If the second word ends
			end = i; // Set end index
			break;
		}
	}

	//word until end of string
	if (end == -1)
		end = strlen(str);
	
	int length = end - start;

	word = (char *)malloc(sizeof(char) * (length + 1));

	//copy word to malloc-ed word
	strncpy(word, str + start, length);
	word[length] = '\0';

	return (word);
}

void	printInitialPrompt(void)
{
	printf("Hello!! Please Authenticate to run server commands\n");
	printf("1. type \"USER\" followed by a space and your username\n");
	printf("2. type \"PASS\" followed by a space and your password\n\n");
	
	printf("\"QUIT\" to close connection at any moment\n");
	printf("Once Authenticated\n");
	printf("this is the list of commands :\n");
	printf("\"STOR\" + space + filename |to send a file to the server\n");
	printf("\"RETR\" + space + filename |to download a file from the server\n");
	printf("\"LIST\" |to list all the files under the current server directory\n");
	printf("\"CWD\" + space + directory |to change the current server directory\n");
	printf("\"PWD\" to display the current server directory\n");
	printf("Add \"!\" before the last three commands to apply them locally\n\n");
}
