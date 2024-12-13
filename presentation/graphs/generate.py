#!/bin/python3
import os
import subprocess

def convert_dot_to_pdf():
    """
    Converts all .dot files in the current directory to PDF using the Graphviz 'dot' tool.
    """
    # Get the current working directory
    cwd = os.getcwd()

    # List all files in the directory
    files = os.listdir(cwd)

    # Filter for .dot files
    dot_files = [file for file in files if file.endswith('.dot')]

    if not dot_files:
        print("No .dot files found in the current directory.")
        return

    # Create output directory if it doesn't exist
    output_dir = os.path.join(cwd, "output_pdfs")
    os.makedirs(output_dir, exist_ok=True)

    for dot_file in dot_files:
        # Construct input and output file paths
        input_path = os.path.join(cwd, dot_file)
        output_file = os.path.splitext(dot_file)[0] + ".pdf"
        output_path = os.path.join(output_dir, output_file)
        if dot_file == 'boruvka_step_1.dot' or dot_file == 'boruvka_step_2.dot':
            command = ["dot", "-Tpdf", "-Kcirco", input_path, "-o", output_path]
        else:
            command = ["dot", "-Tpdf", input_path, "-o", output_path]

        # Generate the PDF using the 'dot' command
        try:
            subprocess.run(command, check=True)
            print(f"Converted {dot_file} to {output_path}")
        except subprocess.CalledProcessError as e:
            print(f"Error converting {dot_file}: {e}")

if __name__ == "__main__":
    convert_dot_to_pdf()
