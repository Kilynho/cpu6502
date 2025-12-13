# GitHub Search Query

## Original Query

```
repo:${owner}/${repository} is:open author:${user}
```

This query searches for open pull requests in a repository authored by a specific user.

**Variables:**
- `${owner}`: The repository owner (user or organization name)
- `${repository}`: The repository name
- `${user}`: The GitHub username of the PR author

## Modified Query to Include Copilot-Created Pull Requests

```
repo:${owner}/${repository} is:open author:${user}
```

**Change:** No change needed. When GitHub Copilot creates PRs "on your behalf", they are authored by you (the user), so the original query already includes them.

**To show ONLY Copilot-created PRs:**
```
repo:${owner}/${repository} is:open author:${user} head:copilot/
```

This filters for PRs from branches starting with `copilot/`, which is the pattern used by GitHub Copilot.
