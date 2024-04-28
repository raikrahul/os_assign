#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char buf[512];

//  "a*bc", Text = "aaabc"

int ismatch(char *text, char *pattern)
{
    // Base case: If the pattern is empty, the text must also be empty for a match
    if (*pattern == 0)
        return *text == 0;

    // Check for the wildcard '*'
    if (*pattern && *(pattern + 1) && *(pattern + 1) == '*')
    {
        // Attempt match without using the wildcard
        if (ismatch(text, pattern + 2))
            return 1;

        // If no match, try matching while stretching the wildcard
        while (*text && (*text == *pattern || *pattern == '.'))
        {
            if (ismatch(text + 1, pattern))
                return 1;
            text++;
        }
    }

    // Check for a direct match or a wildcard '.'
    if ((*text && *pattern == '.') || *text == *pattern)
        return ismatch(text + 1, pattern + 1);

    // No match found
    return 0;
}



char *fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;
    int len;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Calculate the length of the filename
    len = strlen(p);

    // If the filename is within the limit, return it as is
    if (len >= DIRSIZ)
        return p;

    // Copy the filename to the buffer and pad with null characters
    for (int i = 0; i < DIRSIZ; i++)
    {
        if (i < len)
        {
            buf[i] = p[i];
        }
        else
        {
            buf[i] = '\0';
        }
    }

    return buf;
}

void find(char *path, char *name)
{
    char *p;
    int fd;
    struct dirent de;
    struct stat st;

    // Open the file or directory
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // Get the status of the file or directory
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        // Check if the file name matches the search term
        if (ismatch(fmtname(path), name) != 0)
        {
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        // Check if the path length is within the buffer size
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            // Skip if the directory entry is not valid or is a special directory
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            // Recursive call to find in the current directory
            find(buf, name);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    find(argv[1], argv[2]);
    exit(0);
}
