# Development workflow

Our workflow is based on the [shared repository model](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/about-collaborative-development-models) and this [Git branching model](https://nvie.com/posts/a-successful-git-branching-model/). Under the `shared repository model`, developers are granted push access to a single shared repository. Feature branches are created when specific changes need to be made, and merged into a shared branch when approved.

## Repository Access

There are two kinds of access role for each DUNE DAQ repo.

* **Maintain**: one or two developers have the "Maintainer" role for the repo
* **Write**: a GitHub team of developers has write access to the repo

A team is defined at the level of [the GitHub DUNE-DAQ organization](https://github.com/DUNE-DAQ), thus being made a member of a team typically grants write access to a number of related repos. A team is usually managed by DAQ working group leaders. Developers obtain write access by being added into a team. Refer to the [List of teams and repositories](team_repos.md) page to find a list of teams and repositories each team has write access to.

## Branches of DAQ repositories

We have four types of branch in our workflow:

* **Common branches** 
    * The default branch of each repository. As of Feb-11-2024 it has only one of two names: either `production/v4` if you're developing for current datataking at `np04` or `develop` if you're doing longer-term development (e.g., OKS changes for the future `v5.0.0` software release). This branch exists permanently and, shared among all developers, is not generally meant to be worked on (i.e., committed to) directly; it can only be updated via Pull Requests (PRs) which require at least one approving review. For the rest of this document both `production/v4` and `develop` will be referred to as `<common branch>`; let that stand in for whichever of the two branches is relevant for your purposes.   
* **Feature branches**
    * Forked off of the `<common branch>`, and where developers are meant to do their work for a specific task. When work on this branch is complete, it is merged into the `<common branch>` branch via a PR.  
* **Release preparation branches** 
    * These are only intended for use if changes need to be made after the initial tags are made for a particular frozen release's release cycle
    * Intended to be forked off the tag, _not_ the `<common branch>`
    * Can only be updated via PRs _with at least one approval review_ before release cut-off time 
    * After the final tag for the frozen release is made, notify the Software Coordination team to merge it into `<common branch>`, along with any special instructions (like if there shouldn't, in fact, be a merge, or if only a subset of the commits on the branch should be merged)
    * Nomenclature: for a given release `fddaq-vX.Y.Z` use `prep-release/fddaq-vX.Y.Z` and similarly `prep-release/nddaq-vX.Y.Z` for `nddaq-vX.Y.Z`
* **Patch branches**  `patch/fddaq-vX.Y.x`/`patch/nddaq-vX.Y.x`**
    * Used for patch frozen releases; these are forked off of the final tags of the frozen releases we're patching
    * Same rules for merging into `<common branch>` apply here as apply to the prep-release branches
    * Nomenclature is the same as for prep release branches, _except_ we leave the patch version a "variable". So, e.g., while a prep release branch for a FD-based `v4.4.0` release would be `prep-release/fddaq-v4.4.0`, if a `v4.4.1` patch released is based off it a patch release branch would be `patch/fddaq-v4.4.x`

## Tags of DAQ repositories

We have two types of tags for DAQ repositories:

* Version tags: 
    * Made by repo maintainers
    * In the format of `vX.Y.Z` where `X`, `Y` and `Z` is a digit for `MAJOR, MINOR, PATCH` version respectively
    * At a minimum, if `X` is not advanced in a newer DAQ release, and a new tag is needed, the minor version `Y` should be advanced.
* DAQ release tags: 
    * Made by the software coordination team;
    * Aliased to a version tag;
    * Nomenclature: `fddaq-vX.Y.Z` (FD packages), `nddaq-vX.Y.Z` (ND packages), `coredaq-vX.Y.Z` (common packages) 

## Release cycle 

We have adopted a three-phased release cycle:
1. Phase-1, active development period
2. Phase-2, testing period
3. Phase-3, post release (patch release) period

### Phase 1 - Active Development Period
 
In this period, developers make frequent updates to the `<common branch>` via pull requests. The workflow is as follows:

1. Create a GitHub Issue in the repo describing the code change. This is optional for small changes.
2. Create a feature branch, preferably containing the GitHub user name of the creator and the Issue (if applicable). E.g. `git checkout <common branch>; git checkout -b dingpf/issue_12_feature_dev_demo`
3. Locally make commits to the feature branch and push it to GitHub; (`git push -u origin dingpf/issue_12_feature_dev_demo`)
4. Create a pull request to the `<common branch>` branch and link the issue to the pull request if applicable
5. Technically, the pull request can be merged without reviews. But it's highly recommended the author request reviews from other developers if the code change is significant.

The active development period comes to an end when the `<common branch>` is ready to be tagged. The procedure for this is described in the next section. It is _highly_ recommended that before this is done the package's codebase is checked for:

1. [compliance with our coding guidelines](https://dune-daq-sw.readthedocs.io/en/latest/packages/styleguide/) -- in particular that `dbt-build` is run with the `--lint` option and no major issues revealed
2. `dbt-clang-format.sh` is run on the codebase so that whitespace formatting is correct
3. If your package is a dependency of another package, a correct set `find_dependency` calls in `cmake/<packagename>Config.cmake.in`. It's often the case that developers update dependencies in `CMakeLists.txt` without making the corresponding update(s) in `cmake/<packagename>Config.cmake.in`.

Details on the first two steps above can be found in the [daq-buildtools documentation](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-buildtools/#useful-build-options). Details on the third step can be found in the [daq-cmake documentation](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#installing-your-project-as-a-local-package).


### Phase 2 - Testing Period

#### Before the testing period starts

Developers need to bump the version of the package on the `<common branch>`. Either on or before the tag collection date, the person in charge of tagging the package (typically the package maintainer, or whoever is marked as such on the tag collector spreadsheet) should do the following:
1. Consult the tag collector spreadsheet to confirm they're assigned as the package tagger, and to confirm the new version number `<X.Y.Z>`. Any disagreement or confusion about either of these should be resolved before the next step. The spreadsheet is by convention linked to [from the top of the "Instructions for setting up a development area" page of the daqconf Wiki](https://github.com/DUNE-DAQ/daqconf/wiki/Instructions-for-setting-up-a-development-software-area)
2. Update the `project(<package name> VERSION <X.Y.Z>)` line at the top of `CMakeLists.txt`.
3. With the `CMakeLists.txt` modification committed on the `<common branch>`, perform an annotated tag on `<common branch>`: `git tag -a v<X.Y.Z> -m "<your initials>: version v<X.Y.Z>"`
4. Push your `<common branch>` branch and your tag to the central repo: `git push origin <common branch>; git push --tags`
5. Mark your package as "Tag Ready" on the tag collector spreadsheet

#### During the testing period

An initial candidate release will be built once the first round of tags are collected. That marks the start of the testing period. Any further changes made during the testing period should be agreed upon and significant - this is not a time for introducing minor new features, as we want to test as consistent a codebase as possible. Changes which do get made will be made to a prep release branch, as described earlier in this document. 

### Phase 3 - Post Release Period

This is marked by the deployment of the release to cvmfs. No changes will be made to the deployed release, but critical bug fixes can be invited into an associated patch release. Changes which do get made will be made to a patch branch, as described earlier in this document. 



## Useful tips


💡 If the targeted branch of a pull request has advanced, please do the following to bring the feature branch in sync before merging the PR:
1. Switch to the targeted branch, and do a `git pull` to make sure it stays in sync with the remote
2. Switch back to the feature branch of the PR, merge the targeted branch into it, e.g. `git merge --no--ff <targeted branch name>`
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
