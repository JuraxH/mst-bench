#!/bin/python3
import os
import requests

def download_github_subdirectory(repo_owner, repo_name, subdirectory, branch="main", output_dir="."):
    """
    Downloads a specific subdirectory and all its files from a GitHub repository.

    Args:
        repo_owner (str): Owner of the GitHub repository.
        repo_name (str): Name of the GitHub repository.
        subdirectory (str): Path to the subdirectory in the repository.
        branch (str): Branch name to download from (default is "main").
        output_dir (str): Local directory to save the downloaded files.

    Returns:
        None
    """
    api_url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/contents/{subdirectory}?ref={branch}"

    response = requests.get(api_url)

    if response.status_code == 200:
        contents = response.json()

        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        for item in contents:
            if item["type"] == "file":
                download_url = item["download_url"]
                file_name = item["name"]
                file_path = os.path.join(output_dir, file_name)

                print(f"Downloading {file_name}...")

                file_response = requests.get(download_url)
                with open(file_path, "wb") as f:
                    f.write(file_response.content)

            elif item["type"] == "dir":
                new_subdirectory = os.path.join(subdirectory, item["name"])
                new_output_dir = os.path.join(output_dir, item["name"])
                download_github_subdirectory(repo_owner, repo_name, new_subdirectory, branch, new_output_dir)

    else:
        print(f"Failed to fetch contents of {subdirectory}: {response.status_code}, {response.text}")

# Example usage
repo_owner = "JuraxH"
repo_name = "mst-bench"
subdirectory = "graphs/random"
branch = "main"
output_dir = "./random_graphs"

download_github_subdirectory(repo_owner, repo_name, subdirectory, branch, output_dir)
