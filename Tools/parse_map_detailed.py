#!/usr/bin/env python3
"""
Detailed MAP file parser for MidiCore STM32F407
Extracts all symbols from .bss, .data, and .ccmram sections with precise size information
"""

import sys
import re
from collections import defaultdict
from pathlib import Path

class DetailedMapParser:
    def __init__(self, map_file_path):
        self.map_file_path = map_file_path
        self.symbols = []
        self.by_module = defaultdict(lambda: {'bss': 0, 'data': 0, 'ccmram': 0, 'total': 0})
        self.by_file = defaultdict(lambda: {'bss': 0, 'data': 0, 'ccmram': 0, 'total': 0})
        self.total_bss = 0
        self.total_data = 0
        self.total_ccmram = 0
        
    def parse(self):
        """Main parsing function"""
        print(f"Parsing map file: {self.map_file_path}")
        
        with open(self.map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Extract section totals first
        self._extract_section_totals(content)
        
        # Parse each section
        self._parse_section(content, '.bss', 'bss')
        self._parse_section(content, '.data', 'data')
        self._parse_section(content, '.ccmram', 'ccmram')
        
        # Calculate totals
        for module in self.by_module:
            self.by_module[module]['total'] = (
                self.by_module[module]['bss'] + 
                self.by_module[module]['data'] + 
                self.by_module[module]['ccmram']
            )
        
        for file in self.by_file:
            self.by_file[file]['total'] = (
                self.by_file[file]['bss'] + 
                self.by_file[file]['data'] + 
                self.by_file[file]['ccmram']
            )
    
    def _extract_section_totals(self, content):
        """Extract total size of each memory section"""
        # Pattern: .bss            0x200004f8    0x228a4
        bss_match = re.search(r'^\.bss\s+0x[0-9a-f]+\s+0x([0-9a-f]+)', content, re.MULTILINE)
        if bss_match:
            self.total_bss = int(bss_match.group(1), 16)
        
        data_match = re.search(r'^\.data\s+0x[0-9a-f]+\s+0x([0-9a-f]+)', content, re.MULTILINE)
        if data_match:
            self.total_data = int(data_match.group(1), 16)
        
        ccmram_match = re.search(r'^\.ccmram\s+0x[0-9a-f]+\s+0x([0-9a-f]+)', content, re.MULTILINE)
        if ccmram_match:
            self.total_ccmram = int(ccmram_match.group(1), 16)
    
    def _parse_section(self, content, section_name, section_type):
        """Parse a specific memory section"""
        # Find the section start
        section_start = content.find(f'\n{section_name} ')
        if section_start == -1:
            print(f"Warning: Section {section_name} not found")
            return
        
        # Find the section end (next major section or end of file)
        section_end = len(content)
        for next_section in ['\n.rodata', '\n.ARM.', '\n.init_', '\n.fini_', '\nLINKER SCRIPT', '\n/DISCARD/', '\nOUTPUT']:
            pos = content.find(next_section, section_start + 100)
            if pos != -1 and pos < section_end:
                section_end = pos
        
        section_content = content[section_start:section_end]
        
        # Pattern to match symbol lines with address and size
        # Format:  .bss.symbol_name
        #                 0x20001234     0x100 ./path/to/file.o
        # or single line: .bss.symbol_name 0x20001234     0x100 ./path/to/file.o
        
        lines = section_content.split('\n')
        i = 0
        while i < len(lines):
            line = lines[i]
            
            # Skip empty lines and non-symbol lines
            if not line.strip() or '*fill*' in line or line.startswith(' ' * 30):
                i += 1
                continue
            
            # Check for subsection line (starts with space and dot)
            # Format:  .bss.symbol_name  or  .bss.symbol_name 0xaddr 0xsize source.o
            match = re.match(r'^\s+(\.(?:bss|data|ccmram)[^\s]*)\s*(?:0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(.+))?$', line)
            if match:
                symbol_name = match.group(1)
                address = match.group(2)
                size_hex = match.group(3)
                source = match.group(4)
                
                # If size is on this line, process it
                if size_hex and source:
                    size = int(size_hex, 16)
                    if size > 0:
                        self._add_symbol(symbol_name, address, size, source, section_type)
                # Otherwise, check next line for address/size/source
                elif i + 1 < len(lines):
                    next_line = lines[i + 1]
                    next_match = re.match(r'^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(.+)$', next_line)
                    if next_match:
                        address = next_match.group(1)
                        size_hex = next_match.group(2)
                        source = next_match.group(3)
                        size = int(size_hex, 16)
                        if size > 0:
                            self._add_symbol(symbol_name, address, size, source, section_type)
                        i += 1  # Skip the next line since we processed it
            
            i += 1
    
    def _add_symbol(self, symbol_name, address, size, source, section_type):
        """Add a symbol to our tracking structures"""
        # Clean up symbol name
        symbol_clean = symbol_name.replace('.bss.', '').replace('.data.', '').replace('.ccmram.', '')
        
        # Clean up source path
        source_clean = source.strip()
        
        # Extract module and file
        module = self._extract_module(source_clean)
        file_name = self._extract_filename(source_clean)
        
        # Store symbol
        self.symbols.append({
            'name': symbol_clean,
            'section': section_type,
            'address': f"0x{address}" if address else "N/A",
            'size': size,
            'source': source_clean,
            'module': module,
            'file': file_name
        })
        
        # Update counters
        self.by_module[module][section_type] += size
        self.by_file[file_name][section_type] += size
    
    def _extract_module(self, source_path):
        """Extract module name from source path"""
        if './Services/' in source_path:
            match = re.search(r'\./Services/([^/]+)', source_path)
            return f"Services/{match.group(1)}" if match else "Services/unknown"
        elif './App/' in source_path:
            return "App"
        elif './Hal/' in source_path:
            match = re.search(r'\./Hal/([^/]+)', source_path)
            return f"Hal/{match.group(1)}" if match else "Hal"
        elif './Core/' in source_path:
            return "Core"
        elif './Middlewares/' in source_path:
            if 'FreeRTOS' in source_path:
                return "Middlewares/FreeRTOS"
            elif 'USB' in source_path:
                return "Middlewares/USB"
            return "Middlewares/Other"
        elif './Drivers/' in source_path:
            return "Drivers"
        elif './FATFS/' in source_path:
            return "FATFS"
        elif './USB_' in source_path:
            if 'USB_DEVICE' in source_path:
                return "USB_DEVICE"
            elif 'USB_HOST' in source_path:
                return "USB_HOST"
            return "USB"
        else:
            return "Other/LibC"
    
    def _extract_filename(self, source_path):
        """Extract just the filename from source path"""
        if '.o' in source_path:
            match = re.search(r'([^/\\]+\.o)', source_path)
            return match.group(1) if match else source_path
        return source_path
    
    def print_summary(self):
        """Print memory summary"""
        print("\n" + "="*100)
        print("MIDICORE STM32F407 - COMPREHENSIVE RAM USAGE ANALYSIS")
        print("="*100)
        
        print(f"\n{'MEMORY SECTION':<30} {'SIZE (bytes)':>15} {'SIZE (KB)':>12} {'% of Limit':>12}")
        print("-"*100)
        
        ram_limit = 128 * 1024
        ccmram_limit = 64 * 1024
        
        ram_total = self.total_bss + self.total_data
        
        print(f"{'  .bss (uninitialized)':<30} {self.total_bss:>15,} {self.total_bss/1024:>11.2f} {100*self.total_bss/ram_limit:>11.1f}%")
        print(f"{'  .data (initialized)':<30} {self.total_data:>15,} {self.total_data/1024:>11.2f} {100*self.total_data/ram_limit:>11.1f}%")
        print(f"{'  ---':<30} {'-'*15:>15} {'-'*12:>12} {'-'*12:>12}")
        print(f"{'  RAM TOTAL':<30} {ram_total:>15,} {ram_total/1024:>11.2f} {100*ram_total/ram_limit:>11.1f}%")
        print(f"{'  RAM Limit (STM32F407VG)':<30} {ram_limit:>15,} {ram_limit/1024:>11.2f} {'100.0%':>12}")
        
        if ram_total > ram_limit:
            overflow = ram_total - ram_limit
            print(f"{'  ⚠️  RAM OVERFLOW':<30} {overflow:>15,} {overflow/1024:>11.2f} {100*overflow/ram_limit:>11.1f}%")
        else:
            headroom = ram_limit - ram_total
            print(f"{'  ✓ RAM Available':<30} {headroom:>15,} {headroom/1024:>11.2f} {100*headroom/ram_limit:>11.1f}%")
        
        print()
        print(f"{'  .ccmram (Core-Coupled)':<30} {self.total_ccmram:>15,} {self.total_ccmram/1024:>11.2f} {100*self.total_ccmram/ccmram_limit:>11.1f}%")
        print(f"{'  CCMRAM Limit':<30} {ccmram_limit:>15,} {ccmram_limit/1024:>11.2f} {'100.0%':>12}")
        
        if self.total_ccmram > ccmram_limit:
            overflow = self.total_ccmram - ccmram_limit
            print(f"{'  ⚠️  CCMRAM OVERFLOW':<30} {overflow:>15,} {overflow/1024:>11.2f}")
        else:
            headroom = ccmram_limit - self.total_ccmram
            print(f"{'  ✓ CCMRAM Available':<30} {headroom:>15,} {headroom/1024:>11.2f}")
        
        print()
        grand_total = ram_total + self.total_ccmram
        total_limit = ram_limit + ccmram_limit
        print(f"{'  GRAND TOTAL':<30} {grand_total:>15,} {grand_total/1024:>11.2f} {100*grand_total/total_limit:>11.1f}%")
        print(f"{'  Total Limit':<30} {total_limit:>15,} {total_limit/1024:>11.2f} {'100.0%':>12}")
        
    def print_top_modules(self, n=25):
        """Print top N modules by memory usage"""
        print("\n" + "="*100)
        print(f"TOP {n} MEMORY-INTENSIVE MODULES")
        print("="*100)
        print(f"\n{'Module':<40} {'BSS':>12} {'Data':>12} {'CCMRAM':>12} {'Total':>12} {'% Total':>8}")
        print("-"*100)
        
        sorted_modules = sorted(self.by_module.items(), key=lambda x: x[1]['total'], reverse=True)
        grand_total = self.total_bss + self.total_data + self.total_ccmram
        
        for module, sizes in sorted_modules[:n]:
            pct = 100 * sizes['total'] / grand_total if grand_total > 0 else 0
            print(f"{module:<40} {sizes['bss']:>10,} B {sizes['data']:>10,} B {sizes['ccmram']:>10,} B {sizes['total']:>10,} B {pct:>7.1f}%")
    
    def print_top_files(self, n=40):
        """Print top N files by memory usage"""
        print("\n" + "="*100)
        print(f"TOP {n} FILES BY MEMORY USAGE")
        print("="*100)
        print(f"\n{'File':<50} {'BSS':>12} {'Data':>12} {'CCMRAM':>12} {'Total':>12}")
        print("-"*100)
        
        sorted_files = sorted(self.by_file.items(), key=lambda x: x[1]['total'], reverse=True)
        
        for file, sizes in sorted_files[:n]:
            print(f"{file:<50} {sizes['bss']:>10,} B {sizes['data']:>10,} B {sizes['ccmram']:>10,} B {sizes['total']:>10,} B")
    
    def print_large_symbols(self, min_size=1000):
        """Print symbols larger than min_size"""
        print("\n" + "="*100)
        print(f"LARGE SYMBOLS (≥ {min_size} bytes)")
        print("="*100)
        print(f"\n{'Symbol':<50} {'Section':<8} {'Size':>12} {'Module':<40}")
        print("-"*100)
        
        large_symbols = [s for s in self.symbols if s['size'] >= min_size]
        large_symbols.sort(key=lambda x: x['size'], reverse=True)
        
        for sym in large_symbols:
            print(f"{sym['name']:<50} {sym['section'].upper():<8} {sym['size']:>10,} B {sym['module']:<40}")
    
    def find_duplicates(self):
        """Find potential duplicate symbols across files"""
        print("\n" + "="*100)
        print("POTENTIAL DUPLICATE SYMBOLS")
        print("="*100)
        
        # Group by symbol name
        by_name = defaultdict(list)
        for sym in self.symbols:
            # Ignore very small symbols and numbered variants
            if sym['size'] < 100:
                continue
            # Remove numeric suffixes like .0, .1, etc.
            base_name = re.sub(r'\.\d+$', '', sym['name'])
            by_name[base_name].append(sym)
        
        # Find names that appear in multiple files
        duplicates = {name: syms for name, syms in by_name.items() if len(syms) > 1}
        
        if not duplicates:
            print("\n✓ No significant duplicate symbols found (symbols < 100 bytes excluded)")
            return
        
        print(f"\n{'Symbol':<40} {'Count':>8} {'Total Size':>15} {'Files'}")
        print("-"*100)
        
        for name, syms in sorted(duplicates.items(), key=lambda x: sum(s['size'] for s in x[1]), reverse=True):
            total_size = sum(s['size'] for s in syms)
            files = ', '.join(set(s['file'] for s in syms))
            print(f"{name:<40} {len(syms):>8} {total_size:>13,} B {files[:50]}")
    
    def export_csv(self, prefix="ram_analysis"):
        """Export all data to CSV files"""
        # Modules CSV
        with open(f"{prefix}_modules.csv", 'w') as f:
            f.write("Module,BSS,Data,CCMRAM,Total,Percent\n")
            sorted_modules = sorted(self.by_module.items(), key=lambda x: x[1]['total'], reverse=True)
            total = self.total_bss + self.total_data + self.total_ccmram
            for module, sizes in sorted_modules:
                pct = 100 * sizes['total'] / total if total > 0 else 0
                f.write(f'"{module}",{sizes["bss"]},{sizes["data"]},{sizes["ccmram"]},{sizes["total"]},{pct:.2f}\n')
        
        # Files CSV
        with open(f"{prefix}_files.csv", 'w') as f:
            f.write("File,BSS,Data,CCMRAM,Total\n")
            sorted_files = sorted(self.by_file.items(), key=lambda x: x[1]['total'], reverse=True)
            for file, sizes in sorted_files:
                f.write(f'"{file}",{sizes["bss"]},{sizes["data"]},{sizes["ccmram"]},{sizes["total"]}\n')
        
        # All symbols CSV
        with open(f"{prefix}_all_symbols.csv", 'w') as f:
            f.write("Symbol,Section,Size,Module,File,Address,Source\n")
            sorted_symbols = sorted(self.symbols, key=lambda x: x['size'], reverse=True)
            for sym in sorted_symbols:
                f.write(f'"{sym["name"]}",{sym["section"]},{sym["size"]},"{sym["module"]}","{sym["file"]}",{sym["address"]},"{sym["source"]}"\n')
        
        print(f"\n✓ CSV files exported:")
        print(f"  • {prefix}_modules.csv")
        print(f"  • {prefix}_files.csv")
        print(f"  • {prefix}_all_symbols.csv")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 parse_map_detailed.py <map_file> [output_prefix]")
        sys.exit(1)
    
    map_file = sys.argv[1]
    prefix = sys.argv[2] if len(sys.argv) > 2 else "ram_analysis"
    
    if not Path(map_file).exists():
        print(f"Error: Map file not found: {map_file}")
        sys.exit(1)
    
    parser = DetailedMapParser(map_file)
    parser.parse()
    parser.print_summary()
    parser.print_top_modules(25)
    parser.print_top_files(40)
    parser.print_large_symbols(1000)
    parser.find_duplicates()
    parser.export_csv(prefix)
    
    print("\n" + "="*100)
    print("ANALYSIS COMPLETE - See CSV files for full details")
    print("="*100 + "\n")

if __name__ == "__main__":
    main()
