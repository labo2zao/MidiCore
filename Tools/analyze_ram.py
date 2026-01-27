#!/usr/bin/env python3
"""
RAM Analysis Tool for MidiCore STM32F407
Analyzes .map file to extract detailed RAM usage per module, file, and symbol
"""

import sys
import re
from collections import defaultdict
from pathlib import Path

class RAMAnalyzer:
    def __init__(self, map_file_path):
        self.map_file_path = map_file_path
        self.bss_symbols = []
        self.data_symbols = []
        self.ccmram_symbols = []
        self.by_module = defaultdict(lambda: {'bss': 0, 'data': 0, 'ccmram': 0})
        self.by_file = defaultdict(lambda: {'bss': 0, 'data': 0, 'ccmram': 0})
        self.total_bss = 0
        self.total_data = 0
        self.total_ccmram = 0
        
    def parse_map_file(self):
        """Parse the linker map file to extract memory sections"""
        print(f"Parsing map file: {self.map_file_path}")
        
        with open(self.map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Find .bss section
        bss_match = re.search(r'^\s*\.bss\s+0x[0-9a-f]+\s+0x([0-9a-f]+)', content, re.MULTILINE)
        if bss_match:
            self.total_bss = int(bss_match.group(1), 16)
            
        # Find .data section
        data_match = re.search(r'^\s*\.data\s+0x[0-9a-f]+\s+0x([0-9a-f]+)', content, re.MULTILINE)
        if data_match:
            self.total_data = int(data_match.group(1), 16)
            
        # Find CCMRAM section
        ccmram_match = re.search(r'^\s*\.ccmram\s+0x[0-9a-f]+\s+0x([0-9a-f]+)', content, re.MULTILINE)
        if ccmram_match:
            self.total_ccmram = int(ccmram_match.group(1), 16)
        
        # Extract symbols from sections
        self._extract_section_symbols(content, '.bss', self.bss_symbols)
        self._extract_section_symbols(content, '.data', self.data_symbols)
        self._extract_section_symbols(content, '.ccmram', self.ccmram_symbols)
        
    def _extract_section_symbols(self, content, section_name, symbol_list):
        """Extract all symbols from a specific memory section"""
        # Pattern to match symbol lines in map file
        # Format:  .section.name  0xaddress  0xsize  source.o
        pattern = rf'^\s+({re.escape(section_name)}[^\s]*)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(.*?)$'
        
        for match in re.finditer(pattern, content, re.MULTILINE):
            subsection = match.group(1)
            address = match.group(2)
            size_hex = match.group(3)
            source = match.group(4).strip()
            
            size = int(size_hex, 16)
            if size > 0:
                symbol_info = {
                    'subsection': subsection,
                    'address': f"0x{address}",
                    'size': size,
                    'source': source
                }
                symbol_list.append(symbol_info)
                
                # Categorize by module and file
                module = self._extract_module_name(source)
                file_name = self._extract_file_name(source)
                
                if section_name == '.bss':
                    self.by_module[module]['bss'] += size
                    self.by_file[file_name]['bss'] += size
                elif section_name == '.data':
                    self.by_module[module]['data'] += size
                    self.by_file[file_name]['data'] += size
                elif section_name == '.ccmram':
                    self.by_module[module]['ccmram'] += size
                    self.by_file[file_name]['ccmram'] += size
    
    def _extract_module_name(self, source_path):
        """Extract module name from source file path"""
        # Match patterns like ./Services/module_name/ or ./App/ or ./Hal/
        if 'Services/' in source_path:
            match = re.search(r'Services/([^/]+)', source_path)
            return f"Services/{match.group(1)}" if match else "Services/unknown"
        elif 'App/' in source_path:
            return "App"
        elif 'Hal/' in source_path:
            match = re.search(r'Hal/([^/]+)', source_path)
            return f"Hal/{match.group(1)}" if match else "Hal/unknown"
        elif 'Core/' in source_path:
            return "Core"
        elif 'Middlewares/' in source_path:
            return "Middlewares"
        elif 'Drivers/' in source_path:
            return "Drivers"
        elif 'FATFS/' in source_path:
            return "FATFS"
        elif 'USB_' in source_path:
            return "USB"
        else:
            return "Other"
    
    def _extract_file_name(self, source_path):
        """Extract file name from source path"""
        if '.o' in source_path:
            # Extract just the .o filename
            match = re.search(r'([^/\\]+\.o)', source_path)
            return match.group(1) if match else source_path
        return source_path
    
    def print_summary(self):
        """Print overall memory summary"""
        print("\n" + "="*80)
        print("RAM USAGE SUMMARY - MidiCore STM32F407")
        print("="*80)
        print(f"\nTotal Memory Allocation:")
        print(f"  .bss (uninitialized):  {self.total_bss:>10,} bytes ({self.total_bss/1024:>7.1f} KB)")
        print(f"  .data (initialized):   {self.total_data:>10,} bytes ({self.total_data/1024:>7.1f} KB)")
        print(f"  RAM Total:             {self.total_bss + self.total_data:>10,} bytes ({(self.total_bss + self.total_data)/1024:>7.1f} KB)")
        print(f"\n  .ccmram (special):     {self.total_ccmram:>10,} bytes ({self.total_ccmram/1024:>7.1f} KB)")
        print(f"\n  Grand Total:           {self.total_bss + self.total_data + self.total_ccmram:>10,} bytes ({(self.total_bss + self.total_data + self.total_ccmram)/1024:>7.1f} KB)")
        
        # STM32F407VG limits
        ram_limit = 128 * 1024
        ccmram_limit = 64 * 1024
        total_limit = ram_limit + ccmram_limit
        
        ram_used = self.total_bss + self.total_data
        ram_percent = (ram_used / ram_limit) * 100
        ccmram_percent = (self.total_ccmram / ccmram_limit) * 100
        total_percent = ((ram_used + self.total_ccmram) / total_limit) * 100
        
        print(f"\nSTM32F407VG Memory Limits:")
        print(f"  RAM Limit:             {ram_limit:>10,} bytes ({ram_limit/1024:>7.1f} KB)")
        print(f"  CCMRAM Limit:          {ccmram_limit:>10,} bytes ({ccmram_limit/1024:>7.1f} KB)")
        print(f"  Total Limit:           {total_limit:>10,} bytes ({total_limit/1024:>7.1f} KB)")
        
        print(f"\nMemory Usage:")
        print(f"  RAM:        {ram_percent:>6.1f}% ({ram_used:>10,} / {ram_limit:>10,} bytes)")
        print(f"  CCMRAM:     {ccmram_percent:>6.1f}% ({self.total_ccmram:>10,} / {ccmram_limit:>10,} bytes)")
        print(f"  Total:      {total_percent:>6.1f}% ({ram_used + self.total_ccmram:>10,} / {total_limit:>10,} bytes)")
        
        if ram_used > ram_limit:
            print(f"\n  ⚠️  RAM OVERFLOW: +{ram_used - ram_limit:,} bytes over limit!")
        else:
            print(f"\n  ✓ RAM OK: {ram_limit - ram_used:,} bytes available")
            
        if self.total_ccmram > ccmram_limit:
            print(f"  ⚠️  CCMRAM OVERFLOW: +{self.total_ccmram - ccmram_limit:,} bytes over limit!")
        else:
            print(f"  ✓ CCMRAM OK: {ccmram_limit - self.total_ccmram:,} bytes available")
    
    def print_top_modules(self, top_n=20):
        """Print top N modules by RAM usage"""
        print("\n" + "="*80)
        print(f"TOP {top_n} MODULES BY RAM USAGE")
        print("="*80)
        print(f"\n{'Module':<40} {'BSS':>12} {'Data':>12} {'CCMRAM':>12} {'Total':>12}")
        print("-"*80)
        
        # Sort by total RAM usage
        sorted_modules = sorted(self.by_module.items(), 
                               key=lambda x: x[1]['bss'] + x[1]['data'] + x[1]['ccmram'],
                               reverse=True)
        
        for i, (module, sizes) in enumerate(sorted_modules[:top_n], 1):
            total = sizes['bss'] + sizes['data'] + sizes['ccmram']
            print(f"{module:<40} {sizes['bss']:>10,} B {sizes['data']:>10,} B {sizes['ccmram']:>10,} B {total:>10,} B")
    
    def print_top_files(self, top_n=30):
        """Print top N files by RAM usage"""
        print("\n" + "="*80)
        print(f"TOP {top_n} FILES BY RAM USAGE")
        print("="*80)
        print(f"\n{'File':<50} {'BSS':>12} {'Data':>12} {'Total':>12}")
        print("-"*80)
        
        # Sort by total RAM usage
        sorted_files = sorted(self.by_file.items(), 
                             key=lambda x: x[1]['bss'] + x[1]['data'] + x[1]['ccmram'],
                             reverse=True)
        
        for i, (file, sizes) in enumerate(sorted_files[:top_n], 1):
            total = sizes['bss'] + sizes['data'] + sizes['ccmram']
            print(f"{file:<50} {sizes['bss']:>10,} B {sizes['data']:>10,} B {total:>10,} B")
    
    def print_large_symbols(self, min_size=1000):
        """Print all symbols larger than min_size bytes"""
        print("\n" + "="*80)
        print(f"LARGE SYMBOLS (> {min_size} bytes)")
        print("="*80)
        
        # Combine all symbols
        all_symbols = []
        for sym in self.bss_symbols:
            if sym['size'] >= min_size:
                all_symbols.append(('BSS', sym))
        for sym in self.data_symbols:
            if sym['size'] >= min_size:
                all_symbols.append(('DATA', sym))
        for sym in self.ccmram_symbols:
            if sym['size'] >= min_size:
                all_symbols.append(('CCMRAM', sym))
        
        # Sort by size
        all_symbols.sort(key=lambda x: x[1]['size'], reverse=True)
        
        print(f"\n{'Section':<10} {'Size':>12} {'Symbol':<60} {'Source':<40}")
        print("-"*140)
        
        for section, sym in all_symbols:
            # Extract symbol name from subsection
            symbol_name = sym['subsection'].split('.')[-1] if '.' in sym['subsection'] else sym['subsection']
            source_short = sym['source'][-40:] if len(sym['source']) > 40 else sym['source']
            print(f"{section:<10} {sym['size']:>10,} B {symbol_name:<60} {source_short:<40}")
    
    def export_csv(self, output_prefix="ram_analysis"):
        """Export analysis results to CSV files"""
        # Export modules
        with open(f"{output_prefix}_by_module.csv", 'w') as f:
            f.write("Module,BSS (bytes),Data (bytes),CCMRAM (bytes),Total (bytes)\n")
            sorted_modules = sorted(self.by_module.items(), 
                                   key=lambda x: x[1]['bss'] + x[1]['data'] + x[1]['ccmram'],
                                   reverse=True)
            for module, sizes in sorted_modules:
                total = sizes['bss'] + sizes['data'] + sizes['ccmram']
                f.write(f"{module},{sizes['bss']},{sizes['data']},{sizes['ccmram']},{total}\n")
        
        # Export files
        with open(f"{output_prefix}_by_file.csv", 'w') as f:
            f.write("File,BSS (bytes),Data (bytes),CCMRAM (bytes),Total (bytes)\n")
            sorted_files = sorted(self.by_file.items(), 
                                 key=lambda x: x[1]['bss'] + x[1]['data'] + x[1]['ccmram'],
                                 reverse=True)
            for file, sizes in sorted_files:
                total = sizes['bss'] + sizes['data'] + sizes['ccmram']
                f.write(f"{file},{sizes['bss']},{sizes['data']},{sizes['ccmram']},{total}\n")
        
        # Export large symbols
        with open(f"{output_prefix}_large_symbols.csv", 'w') as f:
            f.write("Section,Size (bytes),Symbol,Source\n")
            all_symbols = []
            for sym in self.bss_symbols + self.data_symbols + self.ccmram_symbols:
                if sym['size'] >= 1000:
                    section = 'BSS' if sym in self.bss_symbols else ('DATA' if sym in self.data_symbols else 'CCMRAM')
                    all_symbols.append((section, sym))
            
            all_symbols.sort(key=lambda x: x[1]['size'], reverse=True)
            for section, sym in all_symbols:
                symbol_name = sym['subsection'].split('.')[-1] if '.' in sym['subsection'] else sym['subsection']
                f.write(f"{section},{sym['size']},\"{symbol_name}\",\"{sym['source']}\"\n")
        
        print(f"\n✓ Exported CSV files:")
        print(f"  - {output_prefix}_by_module.csv")
        print(f"  - {output_prefix}_by_file.csv")
        print(f"  - {output_prefix}_large_symbols.csv")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_ram.py <path_to_map_file> [output_prefix]")
        print("Example: python3 analyze_ram.py Debug/MidiCore.map")
        sys.exit(1)
    
    map_file = sys.argv[1]
    output_prefix = sys.argv[2] if len(sys.argv) > 2 else "ram_analysis"
    
    if not Path(map_file).exists():
        print(f"Error: Map file not found: {map_file}")
        sys.exit(1)
    
    analyzer = RAMAnalyzer(map_file)
    analyzer.parse_map_file()
    analyzer.print_summary()
    analyzer.print_top_modules(top_n=25)
    analyzer.print_top_files(top_n=40)
    analyzer.print_large_symbols(min_size=1000)
    analyzer.export_csv(output_prefix)
    
    print("\n" + "="*80)
    print("Analysis complete!")
    print("="*80)

if __name__ == "__main__":
    main()
