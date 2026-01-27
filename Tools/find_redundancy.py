#!/usr/bin/env python3
"""
Code Redundancy Analyzer for MidiCore
Identifies duplicate functions, similar code patterns, and optimization opportunities
"""

import sys
import os
import re
from collections import defaultdict
from pathlib import Path
import hashlib

class RedundancyAnalyzer:
    def __init__(self, repo_path):
        self.repo_path = Path(repo_path)
        self.functions = []  # List of (file, function_name, function_body, line_count)
        self.function_signatures = defaultdict(list)  # signature -> [(file, func_name)]
        self.similar_functions = []  # List of similar function groups
        
    def analyze_directory(self, directory, extensions=['.c', '.h']):
        """Recursively analyze source files in directory"""
        dir_path = self.repo_path / directory
        if not dir_path.exists():
            print(f"Warning: Directory not found: {dir_path}")
            return
        
        print(f"Analyzing {directory}...")
        for file_path in dir_path.rglob('*'):
            if file_path.suffix in extensions and file_path.is_file():
                self._analyze_file(file_path)
    
    def _analyze_file(self, file_path):
        """Extract functions from a single file"""
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            # Simple pattern to find functions
            # Matches: return_type function_name(params) { ... }
            pattern = r'^\s*(?:static\s+)?(?:inline\s+)?(\w+(?:\s*\*)*)\s+(\w+)\s*\([^)]*\)\s*\{'
            
            matches = list(re.finditer(pattern, content, re.MULTILINE))
            
            for match in matches:
                return_type = match.group(1)
                func_name = match.group(2)
                func_start = match.start()
                
                # Find the end of the function (simple brace matching)
                func_body, line_count = self._extract_function_body(content, func_start)
                
                if func_body and line_count > 5:  # Only consider functions > 5 lines
                    rel_path = file_path.relative_to(self.repo_path)
                    self.functions.append({
                        'file': str(rel_path),
                        'name': func_name,
                        'return_type': return_type,
                        'body': func_body,
                        'line_count': line_count,
                        'hash': hashlib.md5(func_body.encode()).hexdigest()
                    })
                    
                    # Create a signature (normalized function body for similarity detection)
                    signature = self._normalize_function(func_body)
                    sig_hash = hashlib.md5(signature.encode()).hexdigest()
                    self.function_signatures[sig_hash].append({
                        'file': str(rel_path),
                        'name': func_name,
                        'body': func_body,
                        'line_count': line_count
                    })
        
        except Exception as e:
            print(f"Error analyzing {file_path}: {e}")
    
    def _extract_function_body(self, content, start_pos):
        """Extract function body using brace matching"""
        brace_count = 0
        in_function = False
        func_body = []
        line_count = 0
        
        for i in range(start_pos, len(content)):
            char = content[i]
            
            if char == '{':
                brace_count += 1
                in_function = True
            elif char == '}':
                brace_count -= 1
                
            if in_function:
                func_body.append(char)
                if char == '\n':
                    line_count += 1
                
                if brace_count == 0:
                    break
        
        return ''.join(func_body), line_count
    
    def _normalize_function(self, func_body):
        """Normalize function for similarity comparison"""
        # Remove comments
        normalized = re.sub(r'//.*?$', '', func_body, flags=re.MULTILINE)
        normalized = re.sub(r'/\*.*?\*/', '', normalized, flags=re.DOTALL)
        
        # Remove whitespace
        normalized = re.sub(r'\s+', ' ', normalized)
        
        # Remove string literals
        normalized = re.sub(r'"[^"]*"', '""', normalized)
        
        # Remove numbers
        normalized = re.sub(r'\b\d+\b', '0', normalized)
        
        return normalized.strip()
    
    def find_exact_duplicates(self):
        """Find exactly duplicate functions"""
        hash_map = defaultdict(list)
        
        for func in self.functions:
            hash_map[func['hash']].append(func)
        
        duplicates = {h: funcs for h, funcs in hash_map.items() if len(funcs) > 1}
        return duplicates
    
    def find_similar_functions(self, min_similarity=0.8):
        """Find functions with similar signatures"""
        similar = {}
        
        for sig_hash, funcs in self.function_signatures.items():
            if len(funcs) > 1:
                # Calculate total lines for this signature
                total_lines = sum(f['line_count'] for f in funcs)
                similar[sig_hash] = {
                    'functions': funcs,
                    'count': len(funcs),
                    'total_lines': total_lines
                }
        
        return similar
    
    def find_common_patterns(self):
        """Identify common code patterns that could be refactored"""
        patterns = {
            'memset_zero': {'pattern': r'memset\s*\([^,]+,\s*0\s*,\s*sizeof', 'count': 0, 'files': set()},
            'memcpy': {'pattern': r'memcpy\s*\(', 'count': 0, 'files': set()},
            'malloc': {'pattern': r'\bmalloc\s*\(', 'count': 0, 'files': set()},
            'free': {'pattern': r'\bfree\s*\(', 'count': 0, 'files': set()},
            'sprintf': {'pattern': r'\bsprintf\s*\(', 'count': 0, 'files': set()},
            'snprintf': {'pattern': r'\bsnprintf\s*\(', 'count': 0, 'files': set()},
            'strlen': {'pattern': r'\bstrlen\s*\(', 'count': 0, 'files': set()},
            'strcpy': {'pattern': r'\bstrcpy\s*\(', 'count': 0, 'files': set()},
            'strncpy': {'pattern': r'\bstrncpy\s*\(', 'count': 0, 'files': set()},
        }
        
        for func in self.functions:
            for pattern_name, pattern_info in patterns.items():
                if re.search(pattern_info['pattern'], func['body']):
                    pattern_info['count'] += 1
                    pattern_info['files'].add(func['file'])
        
        return patterns
    
    def print_summary(self):
        """Print analysis summary"""
        print("\n" + "="*100)
        print("CODE REDUNDANCY ANALYSIS - MIDICORE")
        print("="*100)
        print(f"\nTotal functions analyzed: {len(self.functions)}")
        print(f"Files analyzed: {len(set(f['file'] for f in self.functions))}")
        
    def print_duplicates(self):
        """Print exact duplicate functions"""
        duplicates = self.find_exact_duplicates()
        
        print("\n" + "="*100)
        print("EXACT DUPLICATE FUNCTIONS")
        print("="*100)
        
        if not duplicates:
            print("\n✓ No exact duplicate functions found")
            return
        
        # Sort by number of duplicates and line count
        sorted_dups = sorted(duplicates.items(), 
                           key=lambda x: (len(x[1]), x[1][0]['line_count']),
                           reverse=True)
        
        print(f"\nFound {len(duplicates)} sets of duplicate functions:\n")
        
        for i, (hash_val, funcs) in enumerate(sorted_dups[:20], 1):  # Top 20
            print(f"\n{i}. Function '{funcs[0]['name']}' ({funcs[0]['line_count']} lines)")
            print(f"   Appears {len(funcs)} times:")
            for func in funcs:
                print(f"     • {func['file']}")
    
    def print_similar(self):
        """Print similar functions that could be deduplicated"""
        similar = self.find_similar_functions()
        
        print("\n" + "="*100)
        print("SIMILAR FUNCTIONS (POTENTIAL DUPLICATES)")
        print("="*100)
        
        if not similar:
            print("\n✓ No similar function patterns found")
            return
        
        # Sort by total lines saved
        sorted_similar = sorted(similar.items(),
                              key=lambda x: x[1]['total_lines'],
                              reverse=True)
        
        print(f"\nFound {len(similar)} sets of similar functions:\n")
        
        for i, (sig_hash, info) in enumerate(sorted_similar[:20], 1):  # Top 20
            print(f"\n{i}. Similar pattern ({info['count']} occurrences, ~{info['total_lines']} total lines):")
            for func in info['functions']:
                print(f"     • {func['name']} in {func['file']} ({func['line_count']} lines)")
    
    def print_patterns(self):
        """Print common code patterns"""
        patterns = self.find_common_patterns()
        
        print("\n" + "="*100)
        print("COMMON CODE PATTERNS")
        print("="*100)
        
        print(f"\n{'Pattern':<20} {'Count':>10} {'Files':>10}")
        print("-"*50)
        
        for pattern_name, info in sorted(patterns.items(), key=lambda x: x[1]['count'], reverse=True):
            if info['count'] > 0:
                print(f"{pattern_name:<20} {info['count']:>10} {len(info['files']):>10}")
    
    def export_report(self, output_file="redundancy_analysis.md"):
        """Export detailed redundancy report"""
        duplicates = self.find_exact_duplicates()
        similar = self.find_similar_functions()
        patterns = self.find_common_patterns()
        
        with open(output_file, 'w') as f:
            f.write("# Code Redundancy Analysis - MidiCore\n\n")
            
            f.write(f"## Summary\n\n")
            f.write(f"- Total functions analyzed: {len(self.functions)}\n")
            f.write(f"- Files analyzed: {len(set(func['file'] for func in self.functions))}\n")
            f.write(f"- Exact duplicates found: {len(duplicates)}\n")
            f.write(f"- Similar function groups: {len(similar)}\n\n")
            
            f.write(f"## Exact Duplicates\n\n")
            if duplicates:
                for hash_val, funcs in sorted(duplicates.items(), 
                                             key=lambda x: len(x[1]), reverse=True):
                    f.write(f"### {funcs[0]['name']} ({funcs[0]['line_count']} lines, {len(funcs)} copies)\n\n")
                    for func in funcs:
                        f.write(f"- {func['file']}\n")
                    f.write("\n")
            
            f.write(f"## Similar Functions\n\n")
            if similar:
                for sig_hash, info in sorted(similar.items(), 
                                            key=lambda x: x[1]['total_lines'], reverse=True)[:30]:
                    f.write(f"### Pattern with {info['count']} occurrences ({info['total_lines']} total lines)\n\n")
                    for func in info['functions']:
                        f.write(f"- {func['name']} in {func['file']} ({func['line_count']} lines)\n")
                    f.write("\n")
            
            f.write(f"## Common Patterns\n\n")
            f.write(f"| Pattern | Count | Files |\n")
            f.write(f"|---------|-------|-------|\n")
            for pattern_name, info in sorted(patterns.items(), key=lambda x: x[1]['count'], reverse=True):
                if info['count'] > 0:
                    f.write(f"| {pattern_name} | {info['count']} | {len(info['files'])} |\n")
        
        print(f"\n✓ Detailed report exported to: {output_file}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 find_redundancy.py <repo_path>")
        sys.exit(1)
    
    repo_path = sys.argv[1]
    
    analyzer = RedundancyAnalyzer(repo_path)
    
    # Analyze key directories
    analyzer.analyze_directory("Services")
    analyzer.analyze_directory("App")
    analyzer.analyze_directory("Hal")
    
    analyzer.print_summary()
    analyzer.print_duplicates()
    analyzer.print_similar()
    analyzer.print_patterns()
    analyzer.export_report()
    
    print("\n" + "="*100)
    print("Analysis complete!")
    print("="*100 + "\n")

if __name__ == "__main__":
    main()
