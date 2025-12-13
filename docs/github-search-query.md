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
repo:${owner}/${repository} is:open (author:${user} OR author:app/github-copilot)
```

**Change:** Added `OR author:app/github-copilot` to also show pull requests created by GitHub Copilot.
