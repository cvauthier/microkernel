#ifndef FILENODE_H
#define FILENODE_H

struct FileNode
{
	char *name;
	int size;
	int pos;
	int inode;
	int nb_childs; // -1 -> fichier ; >= 0 -> répertoire
	int childs_size;
	struct FileNode **childs;
};
typedef struct FileNode FileNode;

FileNode *create_node();
void free_node(FileNode *node);

FileNode *find_child(FileNode *node, const char *name);
void add_new_child(FileNode *node, FileNode *child);
char *get_file_path(char header[512]);
int get_file_size(char header[512]);

FileNode *add_file(FileNode *node, const char *path);
FileNode *parse_tarball(FILE *tarball);
void dump_node(FileNode *node);

#endif
