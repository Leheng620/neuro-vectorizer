import os
import argparse
import csv
import shutil

def parse_args() -> argparse.Namespace:
    """Parse commandline arguments."""
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "dir",
        metavar="DIR",
        help="Path to the directory containing source .c files",
    )
    
    parser.add_argument(
        "--test",
        dest="test",
        action="store_true",
        help="whether extract features from test dataset",
    )

    return parser.parse_args()

def main(args: argparse.Namespace) -> None:
    orig_train_files = [os.path.join(root, name)
        for root, _, files in os.walk(args.dir)
        for name in files
        if name.endswith(".c") and not name.startswith('header.c') 
        and not name.startswith('aux_AST_embedding_code.c')]

    # Remove pre-existing feature file and dir
    if os.path.exists("features.csv"):
        print("Removing pre-existing features.csv...")
        os.remove("features.csv")
    
    with open("features.csv", "w") as f:
        writer = csv.writer(f)
        writer.writerow(["filename", "loopDepth", "BBs", "branches", "totalInsts", "intInsts", "fpInsts", "loads", "stores"])

    for file in orig_train_files:
        file.removesuffix(".c")
        barename = file.removesuffix(".c")
        cmd = "./run.sh " + barename
        os.system(cmd)
    
    try:
        os.mkdir("features")
    except OSError as error:
        print(error)
    if not args.test:
        rename_cmd = "mv features.csv features/train_features.csv"
    else:
        rename_cmd = "mv features.csv features/test_features.csv"
    os.system(rename_cmd)

if __name__ == "__main__":
    main(parse_args())
