// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef INCLUDED_FNGEOLIBUTIL_PATH_H
#define INCLUDED_FNGEOLIBUTIL_PATH_H

#include "ns.h"

#include <string>
#include <vector>
#include <set>

FNGEOLIBUTIL_NAMESPACE_ENTER
{

namespace Path
{
    // ALL of these functions, which take as input an absolute scenegraph
    // locationPath, assume a PROPERLY NORMALIZED scenegraph path:
    // - NO trailing slash '/'
    // - no redundent internal separator (A//B, A/B/, A/./B and A/foo/../B all become A/B.)
    //
    // Properly normalized locations:
    //      /root
    //      /root/world/geo
    //      /root/world/taco/a/b
    //
    // Improperly normalized locations:
    //      /root/
    //      /root/world/
    //      /root/world/../a
    //      /root/world/taco//a/b

    std::string NormalizeAbsPath(const std::string & locationPath);

    /*
    GetLocationParent('/root/world/geo') => '/root/world'
    GetLocationParent('/root') => ''
    */
    std::string GetLocationParent(const std::string & locationPath);

    /*
    GetLeafName('/a/b/c') => 'c'
    */
    std::string GetLeafName(const std::string & locationPath);

    void GetLeafAndParent(std::string & parent,
        std::string & leaf,
        const std::string & locationPath);

    /*
    GetLocationStack('/a/b/c') => ['/a','/a/b','/a/b/c']
    GetLocationStack('/a/b/c/d/e', root='/a/b/c') => ['/a/b/c', '/a/b/c/d', '/a/b/c/d/e']
    GetLocationStack('/a/b/c', root='/a/b/c') => ['/a/b/c']
    */
    void GetLocationStack(std::vector<std::string> & returnStack,
        const std::string & locationPath, const std::string & rootPath=std::string());

    /*
    Is Location A a ancestor of location B
    IsAncestorOrEqual('/root/a','/root/a/b/c') => true
    IsAncestorOrEqual('/root/a','/root/a') => true
    */
    bool IsAncestorOrEqual(const std::string & locA, const std::string & locB);

    /*
    Is Location A a ancestor of location B
    IsAncestor('/root/a','/root/a/b/c') => true
    IsAncestor('/root/a','/root/a') => false
    */
    bool IsAncestor(const std::string & locA, const std::string & locB);

    std::string Join(const std::string & locA, const std::string & locB);

    bool IsRelativePath(const std::string & path);

    /*
    RelativeToAbsPath('/root/world','../') => '/root'
    */
    std::string RelativeToAbsPath(const std::string & rootPath, const std::string & path);

    /*
    Given a rootpath, and either a relative or absolute addl path, create a normalized relative path

    NormalizedRelativePath('/root','/root/world') -> 'world'
    NormalizedRelativePath('/root','/root/world/geo') -> 'world/geo'
    NormalizedRelativePath('/root','/root') -> ''
    NormalizedRelativePath('/root','/notroot') -> EXCEPTION
    NormalizedRelativePath('/root','a/b/c') -> 'a/b/c'

    This will throw an exception if root is not ancestor or equal to path
    */
    std::string NormalizedRelativePath(const std::string & rootpath,
        const std::string & path);

    /*
    Given two absolute paths, create a relative path from rootPath to path
    (even if rootPath is not an ancestor of path).

    RelativePath('/root/world/geo', '/root/world/geo/a') -> 'a'
    RelativePath('/root/world/geo/a', '/root/world/geo') -> '..'
    RelativePath('/root/world/geo/a', '/root/world/geo/b') -> '../b'
    RelativePath('/root/world/geo/a', '/root/world/geo/a') -> ''
    RelativePath('/root/world/geo/a', '/root/world/cam/a') -> '../../cam/a'
    */
    std::string RelativePath(const std::string & rootPath,
        const std::string & path);

    // both testpath and pattern should be absolute scengraph paths, normalized
    // in the style of pystring::normpath_posix

    struct FnMatchInfo
    {
        bool match;
        bool canMatchChildren;
    };

    void FnMatch(FnMatchInfo & matchInfo,
                 const std::string & testpath,
                 const std::string & pattern);

    // both testpath and pattern should be canonical absolute location paths
    // with leading slash, without trailing slash
    void ExactMatch(FnMatchInfo & matchInfo,
                 const std::string & testpath,
                 const std::string & pattern);

    std::string MakeUniqueName(const std::string & baseName,
                               const std::set<std::string> & existingNames);

    std::string MakeSafeIdentifier(const std::string &identifier);
}

}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_PATH_H
