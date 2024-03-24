#include "server.h"

int	countWords(const char *str)
{
	bool	inWord = false;
	int		count = 0;

	while (*str != '\0')
	{
		//urrent character is whitespace
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
		str++; // Move to the next character
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
				count++; // Increment word count when encountering whitespace after a word
				inWord = false;
				if (count == 2)
				{ // Check if this is the second word
					start = i + 1; // Set start index to the character after whitespace
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

int validUser(const char* username)
{
	FILE* file = fopen("users.txt", "r");
	if (file == NULL)
	{
		printf("Error opening file.\n");
		return (-1); //file open failure
	}

	char line[BUFFER_SIZE];
	while (fgets(line, sizeof(line), file)) 
	{
		char *storedUsername = strtok(line, ":");
		if (strcmp(storedUsername, username) == 0)
		{
			fclose(file);
			return (1); //valid user found
		}
	}

	fclose(file);
	return (0); //username not found
}

int validPassword(const char* username, const char* password)
{
	FILE* file = fopen("users.txt", "r");
	if (file == NULL)
	{
		printf("Error opening file.\n");
		return (-1); //file open failure
	}

	char line[BUFFER_SIZE];
	while (fgets(line, sizeof(line), file))
	{
		char* storedUsername = strtok(line, ":");
		char* storedPassword = strtok(NULL, ":");
		storedPassword[strcspn(storedPassword, "\n")] = 0; //remove newline

		if (strcmp(storedUsername, username) == 0)
		{
			if (strcmp(storedPassword, password) == 0)
			{
				fclose(file);
				return (1); //valid password
			}
			else
			{
				fclose(file);
				return (0); //invalid password
			}
		}
	}

	fclose(file);
	return (0); //username not found
}