#!/usr/bin/env python3
"""
RAM Validation Script for MidiCore STM32F407
Parses linker map file and validates RAM usage is within limits
"""

import re
import sys

def parse_map_file(map_path):
    """Parse linker map file and extract memory usage"""
    with open(map_path, 'r') as f:
        content = f.read()
    
    # Find .bss and .data sections
    bss_match = re.search(r'^\.bss\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)', content, re.MULTILINE)
    data_match = re.search(r'^\.data\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)', content, re.MULTILINE)
    ccmram_match = re.search(r'^\.ccmram\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)', content, re.MULTILINE)
    
    if not bss_match or not data_match:
        print("ERROR: Could not find .bss or .data sections in map file")
        return None
    
    bss_size = int(bss_match.group(2), 16)
    data_size = int(data_match.group(2), 16)
    ccmram_size = int(ccmram_match.group(2), 16) if ccmram_match else 0
    
    return {
        'bss': bss_size,
        'data': data_size,
        'ccmram': ccmram_size,
        'total_ram': bss_size + data_size,
        'total_all': bss_size + data_size + ccmram_size
    }

def validate_ram(usage):
    """Validate RAM usage against STM32F407 limits"""
    RAM_LIMIT = 128 * 1024      # 128 KB
    CCMRAM_LIMIT = 64 * 1024    # 64 KB
    TOTAL_LIMIT = 192 * 1024    # 192 KB
    
    results = {
        'ram_ok': usage['total_ram'] <= RAM_LIMIT,
        'ccmram_ok': usage['ccmram'] <= CCMRAM_LIMIT,
        'total_ok': usage['total_all'] <= TOTAL_LIMIT,
        'ram_overflow': usage['total_ram'] - RAM_LIMIT,
        'ccmram_overflow': usage['ccmram'] - CCMRAM_LIMIT,
        'total_overflow': usage['total_all'] - TOTAL_LIMIT,
        'ram_percent': 100 * usage['total_ram'] / RAM_LIMIT,
        'ccmram_percent': 100 * usage['ccmram'] / CCMRAM_LIMIT,
        'total_percent': 100 * usage['total_all'] / TOTAL_LIMIT
    }
    
    return results

def print_report(usage, validation):
    """Print formatted validation report"""
    print("=" * 80)
    print("MidiCore RAM Validation Report")
    print("=" * 80)
    print()
    
    print("Memory Usage:")
    print(f"  .bss:       {usage['bss']:>10,} bytes ({usage['bss']/1024:>7.1f} KB)")
    print(f"  .data:      {usage['data']:>10,} bytes ({usage['data']/1024:>7.1f} KB)")
    print(f"  Total RAM:  {usage['total_ram']:>10,} bytes ({usage['total_ram']/1024:>7.1f} KB)")
    print()
    print(f"  CCMRAM:     {usage['ccmram']:>10,} bytes ({usage['ccmram']/1024:>7.1f} KB)")
    print()
    print(f"  Grand Total:{usage['total_all']:>10,} bytes ({usage['total_all']/1024:>7.1f} KB)")
    print()
    
    print("STM32F407VG Limits:")
    print(f"  RAM:        {128*1024:>10,} bytes (128.0 KB)")
    print(f"  CCMRAM:     {64*1024:>10,} bytes (64.0 KB)")
    print(f"  Total:      {192*1024:>10,} bytes (192.0 KB)")
    print()
    
    print("Validation Results:")
    
    # RAM check
    if validation['ram_ok']:
        print(f"  ✓ RAM:      {usage['total_ram']:,} / {128*1024:,} bytes ({validation['ram_percent']:.1f}%)")
        print(f"             Headroom: {-validation['ram_overflow']:,} bytes ({-validation['ram_overflow']/1024:.1f} KB)")
    else:
        print(f"  ✗ RAM:      {usage['total_ram']:,} / {128*1024:,} bytes ({validation['ram_percent']:.1f}%)")
        print(f"             OVERFLOW: +{validation['ram_overflow']:,} bytes (+{validation['ram_overflow']/1024:.1f} KB) ⚠️")
    print()
    
    # CCMRAM check
    if validation['ccmram_ok']:
        print(f"  ✓ CCMRAM:   {usage['ccmram']:,} / {64*1024:,} bytes ({validation['ccmram_percent']:.1f}%)")
        print(f"             Headroom: {-validation['ccmram_overflow']:,} bytes ({-validation['ccmram_overflow']/1024:.1f} KB)")
    else:
        print(f"  ✗ CCMRAM:   {usage['ccmram']:,} / {64*1024:,} bytes ({validation['ccmram_percent']:.1f}%)")
        print(f"             OVERFLOW: +{validation['ccmram_overflow']:,} bytes (+{validation['ccmram_overflow']/1024:.1f} KB) ⚠️")
    print()
    
    # Total check
    if validation['total_ok']:
        print(f"  ✓ TOTAL:    {usage['total_all']:,} / {192*1024:,} bytes ({validation['total_percent']:.1f}%)")
        print(f"             Headroom: {-validation['total_overflow']:,} bytes ({-validation['total_overflow']/1024:.1f} KB)")
    else:
        print(f"  ✗ TOTAL:    {usage['total_all']:,} / {192*1024:,} bytes ({validation['total_percent']:.1f}%)")
        print(f"             OVERFLOW: +{validation['total_overflow']:,} bytes (+{validation['total_overflow']/1024:.1f} KB) ⚠️")
    print()
    
    print("=" * 80)
    
    # Overall verdict
    if validation['ram_ok'] and validation['ccmram_ok'] and validation['total_ok']:
        print("RESULT: ✅ PASS - Firmware fits within STM32F407 RAM capacity")
        return 0
    else:
        print("RESULT: ❌ FAIL - Firmware exceeds STM32F407 RAM capacity")
        return 1

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 validate_ram.py <path-to-map-file>")
        print("Example: python3 validate_ram.py Debug/MidiCore.map")
        sys.exit(1)
    
    map_path = sys.argv[1]
    
    try:
        usage = parse_map_file(map_path)
        if usage is None:
            sys.exit(1)
        
        validation = validate_ram(usage)
        exit_code = print_report(usage, validation)
        sys.exit(exit_code)
        
    except FileNotFoundError:
        print(f"ERROR: Map file not found: {map_path}")
        sys.exit(1)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
