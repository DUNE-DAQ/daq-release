# Temporary Development workflow for NearDet

We are following the [shared repository model](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/about-collaborative-development-models) and this [Git branching model](https://nvie.com/posts/a-successful-git-branching-model/) at the current stage of the DAQ software development. Under the `shared repository model`, developers are granted push access to a single shared repository. Topic branches are created when changes need to be made by following the `Git branching model`.

## Repository Access

We use [two access roles](https://home.fnal.gov/~dingpf/repo_access_role.png) for each repo.

* Maintain: one or two developers have the "Maintainer" role to each repository;
* Write: a GitHub team of developers.

Teams are entities of the DUNE-DAQ project, thus they can be used across multiple repos.

A team is usually managed by DAQ working group leaders. Developers obtain write access by being added into a team. Refer to the [List of teams and repositories](team_repos.md) page to find a list of teams and repositories each team has write access to.


## Branches of DAQ repositories

* **Long-lived branches: `develop`** (default branch of each repository);
* **`nd-patch/nddaq-vX.Y.Z`** (i.e. `nd-patch/nddaq-v3.2.0`):
  * branch off from a tagged version used by a frozen release;
  * not merged to `develop` unless explicitly required.


## Branch protection rules

* **`develop`** branch: require pull requests. All commits must be made to a non-protected branch and submitted via a pull request before they can be merged.
* **`nd-patch/*`** branches: no protections rules in effect. Strongly suggest to create PRs for extensive changes.

## ND-related Tags of DAQ repositories

We have two types of ND-related tags for DAQ repositories:

* Version tags: 
    * made by repo maintainers
    * in the format of `nd-v<X.Y.Z>` where `X`, `Y` and `Z` is a digit for `MAJOR, MINOR, PATCH` version respectively;
    * at a minimum, if `X` is not advanced in a newer DAQ release, and a new tag is needed, the minor version `Y` should be advanced.
* DAQ release tags: 
    * made by the software coordination team;
    * alias to a version tag;
    * in the format of `nddaq-v<X.X.X>` where X is a digit.

## Release cycle 

### Active Development Period

Developers make frequent updates to the `nd-patch/nddaq-vX.Y.Z` branch, (i.e. `nd-patch/nddaq-v3.2.0`) directly or via pull requests (PRs). The workflow with PRs will be like the following:

1. Create a GitHub issue in the repo describe the code change. This is optional for small changes.
2. Create a topic branch; (`git checkout develop; git checkout -b dingpf/issue_12_feature_dev_demo`)
3. Make code development, commit, and push the topic branch to GitHub; (`git push -u origin dingpf/issue_12_feature_dev_demo`)
4. Create a pull request to the `nd-patch/nddaq-v3.2.0` branch and link the issue to the pull request if one was created in step 1;
5. Technically, the pull request can be merged without reviews. But it's highly recommended the author request reviews from other developers if the code change is significant.

The active development period comes to an end when the `nd-patch` branch is ready to be tagged. The procedure for this is described in the next section. It is _highly_ recommended that before this is done the package's codebase is checked for:

1. [compliance with our coding guidelines](https://dune-daq-sw.readthedocs.io/en/latest/packages/styleguide/) -- in particular that `dbt-build` is run with the `--lint` option and no major issues revealed;
2. `dbt-clang-format.sh` is run on the codebase so that whitespace formatting is correct;
3. If your package is a dependency of another package, a correct set `find_dependency` calls in `cmake/<packagename>Config.cmake.in`. It's often the case that developers update dependencies in `CMakeLists.txt` without making the corresponding update(s) in `cmake/<packagename>Config.cmake.in`.

Details on the first two steps above can be found in the [daq-buildtools documentation](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-buildtools/#useful-build-options). Details on the third step can be found in the [daq-cmake documentation](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#installing-your-project-as-a-local-package).


### Procedure for creating `nd-v<X.Y.Z>` tags

When it is ready to create tags (versions tags `nd-vX.Y.Z`) on `nd-patch/nddaq-vX.Y.Z` branches:

1. Update the `project(<package name> VERSION <X.Y.Z>)` line at the top of `CMakeLists.txt`;
2. With the `CMakeLists.txt` modification committed on `nd-patch/nddaq-v<X.Y.Z>`, create an annotated tag with: `git tag -a nd-v<X.Y.Z> -m "<your initials>: version nd-v<X.Y.Z>"`
3. Push the branch and your tag to the central repo: `git push origin nd-patch/nddaq-v<X.Y.Z>; git push --tags`

### Release test build and ND release tag

Developers should hand over a list of `nd-v<X.Y.Z>` tags to the Software Coordination team before requesting a release-wide test build. Upon successful release build, Software Coordination will create a release-wide tag `nddaq-v<X.Y.Z>` across all DAQ repositories. 

## Useful tips


💡 If the targeted branch of a pull request has advanced, please do the following to bring the feature branch in sync before merging the PR:
1. Switch to the targeted branch, and do a `git pull` to make sure it stays in sync with the remote;
2. Switch back to the feature branch of the PR, merge the targeted branch into it, e.g. `git merge --no--ff <targeted branch name>`;
3. Push the merge to remote, and continue with the PR review/merge process.

:red_circle: Please don't use `git rebase` or `git push --force`. It will likely bring unexpected consequences.

* Using `#Issue_Number` in your commit message will make GitHub add links to the commit on the issue page;
* Use `user/repo#issue_number` to link issues in a different repo, e.g. `DUNE-DAQ/daq-cmake#1`;
* **Use with caution** Delete a git tag: `git tag -d v1.0.1` **Do not delete any tag which might be used by others**;
* Push the deletion of a tag to GitHub: `git push --delete origin v1.0.1` **Do not delete any tag which might be used by others**;
* Delete a remote branch: `git push origin --delete feature/branch_name` (not recommended; recommend to delete branches via GitHub web UI);
* List all tags in GitHub repo: `git ls-remote --tags origin`;
* Fetch all branches and tags: `git fetch --all --tags`.

<!---
## Screenshots of some examples

### Repository access

![repo-access](https://i.imgur.com/ddLJeif.png)

### Branch settings

![branch-settings](https://i.imgur.com/WbBJB86.png)

### Branch protection rules

![branch-protection-rules](https://i.imgur.com/NMp0vMU.png)

### Managing branches

![managing-branches](https://i.imgur.com/d25W5er.png)

### View Network Graph

![network-graph](https://i.imgur.com/ogmjKYr.png)
--->
