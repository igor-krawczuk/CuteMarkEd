#ifndef REPO_H
#define REPO_H
#include <QtCore/qfileinfo.h>
#include <git2.h>

class GitRepo{
private:
    QFileInfo *repo_path;
    git_repository *repo;
    git_index *index;
    static void lib_init();

public:
    GitRepo();
    bool open_or_initialize(const QFileInfo& repo_path);
    /**
     * Updates the index of the active repository (equivalent to git add . in WD)
     *
     */
    bool addAll();
    bool no_init();


    /**
     * Commit staged index changes
     */
    bool commit(const QString& message, const QString& name,const QString& email);
    ~GitRepo();
};

#endif // REPO_H
