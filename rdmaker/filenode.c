#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filenode.h"

FileNode *create_node()
{
	return (FileNode*) calloc(1, sizeof(FileNode));
}

void free_node(FileNode *node)
{
	free(node->name);
	if (node->nb_childs >= 0)
	{
		for (int i = 0 ; i < node->nb_childs ; i++)
			free_node(node->childs[i]);
		free(node->childs);
	}
	free(node);
}

FileNode *find_child(FileNode *node, const char *name)
{
	for (int i = 0 ; i < node->nb_childs ; i++)
	{
		if (strcmp(node->childs[i]->name, name) == 0)
			return node->childs[i];
	}
	return 0;
}

void add_new_child(FileNode *node, FileNode *child)
{
	if (node->nb_childs < 0)
		return;
	
	if (node->childs_size == 0)
	{
		node->childs_size = 2;
		node->childs = (FileNode**) malloc(sizeof(FileNode*) * 2);
		node->childs[0] = child;
		node->childs[1] = 0;
	}
	else 	
	{
		if (node->nb_childs == node->childs_size)
		{
			node->childs = (FileNode**) realloc(node->childs, 2*node->childs_size*sizeof(FileNode*));
			node->childs_size *= 2;
		}
		node->childs[node->nb_childs] = child;
	}
	node->nb_childs++;
}

char *get_file_path(char header[512])
{
	int name_len = 0;
	while (name_len < 100 && header[name_len]) 
		name_len++;
		
	char *path = (char*) malloc(sizeof(char)*(name_len+1));
	memcpy(path, header, name_len);
	path[name_len] = 0;
	return path;
}

int get_file_size(char header[512])
{
	int res = 0;
	for (int i = 0 ; i < 11 ; i++)
		res = 8*res + (int) (header[124+i]-'0');
	return res;
}

FileNode *add_file(FileNode *node, const char *path)
{
	if (path[0] != '/')
		return 0;
	
	FileNode *res = 0;
	char *subpath = strchr(path+1, '/');
	
	if (!subpath || !subpath[1]) // Fin de chemin : fichier ou répertoire
	{
		int name_len = !subpath ? strlen(path) : (subpath-path)-1;
		char *filename = (char*) malloc(sizeof(char)*(name_len+1));
		memcpy(filename, path+1, name_len);
		filename[name_len] = 0;
		
		res = find_child(node, filename);
		if (!res)
		{
			res = create_node();
			res->nb_childs = !subpath ? -1 : 0;
			res->name = filename;
			add_new_child(node, res);
		}
		else
			free(filename);
	}
	else
	{
		int name_len = (subpath-path)-1;
		char *dirname = (char*) malloc(sizeof(char)*(name_len+1));
		memcpy(dirname, path+1, name_len);
		dirname[name_len] = 0;

		FileNode *dir = find_child(node, dirname);
		if (dir)
		{
			res = add_file(dir, subpath);
			free(dirname);
		}
		else
		{
			dir = create_node();
			dir->name = dirname;
			dir->nb_childs = 0;
			add_new_child(node, dir);
			res = add_file(dir, subpath);
		}
	}

	return res;
}

FileNode *parse_tarball(FILE *tarball)
{
	FileNode *root = create_node();
	root->name = (char*) calloc(1,sizeof(char));

	char block[512];
	while (fread(block, 1, 512, tarball))
	{
		char *path = get_file_path(block);

		char *subpath = strchr(path, '/');
		int size = get_file_size(block);
		if (subpath && subpath[1])
		{
			FileNode *node = add_file(root, subpath);
			node->size = size; 
			node->pos = ftell(tarball);
		}

		fseek(tarball, (size+511)/512*512, SEEK_CUR);
		free(path);
	}

	return root;
}

void dump_node(FileNode *node)
{
	printf("--------------------\nNode name :%s\n", node->name);
	printf("Size : %d\nDirectory : %d\n", node->size, node->nb_childs >= 0);
	if (node->nb_childs >= 0)
	{
		printf("Childs :\n");
		for (int i = 0 ; i < node->nb_childs ; i++)
			printf("- %s\n", node->childs[i]->name);
		for (int i = 0 ; i < node->nb_childs ; i++)
			dump_node(node->childs[i]);
	}
}


