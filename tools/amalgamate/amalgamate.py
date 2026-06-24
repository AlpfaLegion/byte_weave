# SPDX-License-Identifier: MIT

import re
import argparse
from pathlib import Path
from collections import deque

class TranslationUnit:
    INCLUDE_PATERN = re.compile(r"^\s*#\s*include\s*[<\"]([^>\"]+)[>\"]")
    PRAGMA_ONCE_PATERN = re.compile(r"^\s*#\s*pragma\s+once\s*$")
    CPP_COMMENT_PATERN = re.compile(r"^\s*//(.*)$")

    def __init__(self, file_path: Path, verbose: bool = True):
        self.file_path: Path = file_path
        self.verbose = verbose
        self.content: str = str()
        self.includes: dict = dict()
        self.lib_includes: list = list()
        self.std_includes: list = list()
        self.other_includes: list = list()
    
    def __read(self):
        with open(self.file_path, 'r') as f:
            lines = f.readlines()
        return lines
    
    def __check_std_include(self):
        return True

    def __parse_include(self, line: str):
        match = TranslationUnit.INCLUDE_PATERN.match(line)
        if match:
            include_file_name = match.group(1)
            subdir = include_file_name.split('/')
            if len(subdir) == 1:
                if self.__check_std_include():
                    self.std_includes.append(include_file_name)
                else:
                    self.other_includes.append(include_file_name)
            else:
                if subdir[0] == "byte_weave":
                    self.lib_includes.append(include_file_name)
                else:
                    self.other_includes.append(include_file_name)
            return True
        return False

    def __parse_pragma(self, line: str):
        match = TranslationUnit.PRAGMA_ONCE_PATERN.match(line)
        if match:
            return True
        return False

    def __parse_cpp_comment(self, line: str):
        match = TranslationUnit.CPP_COMMENT_PATERN.match(line)
        if match:
            comment = match.group(1)

            return True
        return False
            

    def get_lib_includes(self):
        return self.lib_includes
    
    def get_std_includes(self):
        return self.std_includes
    
    def get_other_includes(self):
        return self.lib_includes

    def parse(self):
        lines = self.__read()

        for line in lines:
            sts_comment = self.__parse_cpp_comment(line)
            sts_include = self.__parse_include(line)
            sts_pragma = self.__parse_pragma(line)
            if not (sts_include or sts_pragma or sts_comment):
                self.content += line

class Amalgamate:
    def __init__(self, source_dir: Path, target_file: Path, verbose: bool = True, force_save: bool = False):
        self.source_dir: Path = source_dir
        self.target_file: Path = target_file

        self.source_hpp: Path = self.source_dir / "byte_weave.hpp"
        self.amalgamate: str = str()
        self.verbose: bool = verbose
        self.force_save: bool = force_save

    def __parse(self):
        # make graph nodes
        tu_list = list()
        known_files: set = set()
        tu_by_relpath: dict = dict()
        sys_includes: set = set()

        for path in self.source_dir.rglob("*.hpp"):
            if path.is_file():
                rel = path.relative_to(self.source_dir).as_posix()
                tu = TranslationUnit(path,  self.verbose)

                known_files.add(rel)
                tu_list.append((tu, rel))
                tu_by_relpath[rel] = tu

        missing_includes: list = list()
        adj: dict = dict()
        for tu, rel in tu_list:
            tu.parse()

            lib_incs = tu.get_lib_includes()
            sys_includes.update(tu.get_std_includes())

            adj[rel] = list()
            for inc in lib_incs:
                if inc not in known_files:
                    missing_includes.append(inc)
                else:
                    adj[rel].append(inc)

        if self.verbose:
            if len(missing_includes):
                print("Found missing headers:")
                for inc in missing_includes:
                    print (f"\t{inc}") 

        # topological sort via DFS
        colors = {rel: 0 for _, rel in tu_list} # 0 white 1 - gray 2 - black
        order = []
        for _, start_node in tu_list:
            
            if colors[start_node] != 0:
                continue

            colors[start_node] = 1
            stack: deque = deque((start_node, ))

            while len(stack) != 0:
                u = stack[-1]
                has_unvis_child = False

                for v in adj[u]:
                    if colors[v] == 1:
                        stack_list = list(stack)
                        idx = stack_list.index(v)
                        cycle = stack_list[idx:] + [v]
                        raise ValueError("Find cycle: " + " -> ".join(cycle))
                    
                    elif colors[v] == 0:
                        colors[v] = 1
                        stack.append(v)
                        has_unvis_child = True
                        break
                
                if not has_unvis_child:
                    u = stack.pop()
                    colors[u] = 2
                    order.append(u)

        if self.verbose:
            print("Files merging order:")
            for f in order:
                print(f"\t{f}")
        
        return sys_includes, order, tu_by_relpath
        

    def __amalgamate(self, sys_includes: list, order: list, map_tu: dict):
        self.amalgamate += "// SPDX-License-Identifier: MIT\n"
        self.amalgamate += "// Copyright (c) 2026 Timur Abdullov\n"
        self.amalgamate += "//\n"
        self.amalgamate += "// This file is generated by tools/amalgamate/amalgamate.py\n"
        self.amalgamate += "#pragma once\n\n"
        for inc in sys_includes:
            self.amalgamate += f"#include <{inc}>\n"

        rep = 100
        for v in order:
            tu = map_tu[v]

            if len(tu.content) != 0:
                self.amalgamate += "\n\n"
                left = int(rep / 2 - len(v) / 2)
                comment_header = "//" + ("=" * left ) + f" {v} " + ("=" *(rep - (left + len(v))))
                self.amalgamate += comment_header
                
                if tu.content[0] != '\n':
                    self.amalgamate += '\n'

                self.amalgamate += tu.content

                if tu.content[-1] != '\n':
                    self.amalgamate += '\n'

                self.amalgamate += "//" + ("=" * (rep))

    def __write(self):
        self.target_file.parent.mkdir(parents = True, exist_ok = True)

        if ((self.force_save) or (not self.target_file.exists())):
            with open(self.target_file, 'w') as f:
                f.write(self.amalgamate)
        else:
            raise ValueError("File exists, content not save")

    def run(self):
        try:
            sys_includes, order, tu_by_relpath = self.__parse()
            self.__amalgamate(sys_includes, order, tu_by_relpath)
            self.__write()
            print("Done!!!")
        except Exception as er:
            print(str(er))
        

def main(args):

    source_dir = Path(args.source_dir)
    target_dir = Path(args.target_dir) / "byte_weave" / "byte_weave.hpp"
    verbose = args.verbose
    force_save = args.force_save

    amalgamation = Amalgamate(source_dir, target_dir, verbose, force_save)
    amalgamation.run()

    
if __name__ == "__main__":
    description = "Amalgamate headers files"
    usage =" ".join([
        "python amalgamate.py",
        "-s source dir, include byte_weave",
        "-t target file",
        "[-v] verbose",
        "[-f] force save target file"
    ])
    parser = argparse.ArgumentParser(description = description, usage = usage)
    
    parser.add_argument("-v", "--verbose", default = False, required =  False, action="store_true", help = "Verbose")
    parser.add_argument("-f", "--force_save", default = False, required = False, action="store_true",  help = "Force save target file, rewrite existing target file")
    parser.add_argument("-s", "--source_dir", required = True, help = "Include dir(source code) of byte_weave library")
    parser.add_argument("-t", "--target_dir", required = True, help = "Target dir to save byte_weave.hpp file")
    
    main(parser.parse_args())
