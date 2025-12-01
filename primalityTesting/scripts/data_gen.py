import sympy
import random
import os

# --- CONFIGURATION ---
BIT_LENGTHS = [128, 256, 512, 1024, 2048]
SAMPLES_PER_BATCH = 20  # numbers per bit-length?

# Paths
DATA_DIR = "../data"
os.makedirs(DATA_DIR, exist_ok=True)

def generate_primes(bits, count):
    """Generates 'count' distinct primes of 'bits' length."""
    print(f"[*] Generating {count} Primes of {bits}-bits...")
    primes = set()
    while len(primes) < count:
        # 2**(bits-1) ensures it has the correct number of bits (no leading zeros)
        p = sympy.randprime(2**(bits-1), 2**bits)
        primes.add(p)
    return list(primes)

def generate_composites(bits, count):
    """Generates 'count' composites.
    Strategy: Multiply two smaller primes to create 'Hard' composites 
    that look somewhat random but are definitely composite."""
    print(f"[*] Generating {count} Composites of {bits}-bits...")
    composites = set()
    while len(composites) < count:
        # Multiply two numbers approx half the bit length
        a = sympy.randprime(2**((bits//2)-2), 2**(bits//2))
        b = sympy.randprime(2**((bits//2)-2), 2**(bits//2))
        comp = a * b
        composites.add(comp)
    return list(composites)

def save_to_file(filename, numbers):
    filepath = os.path.join(DATA_DIR, filename)
    with open(filepath, "w") as f:
        for num in numbers:
            f.write(f"{num}\n")
    print(f"    Saved to {filepath}")

def main():
    # 1. Generate Standard Benchmarks (Primes & Composites)
    for bits in BIT_LENGTHS:
        # Primes
        primes = generate_primes(bits, SAMPLES_PER_BATCH)
        save_to_file(f"primes_{bits}.txt", primes)
        
        # Composites
        comps = generate_composites(bits, SAMPLES_PER_BATCH)
        save_to_file(f"composites_{bits}.txt", comps)

    # 2. Carmichael Numbers
    carmichaels = [
        561, 1105, 1729, 2465, 2821, 6601, 8911, 10585, 15841, 29341, 
        41041, 46657, 52633, 62745, 63973, 75361, 101101, 115921, 126217, 
        294409 * 570241 * 1023241  
    ]
    print(f"[*] Saving {len(carmichaels)} Carmichael Numbers...")
    save_to_file("carmichael.txt", carmichaels)

    print("\n--- Data Generation Complete ---")

if __name__ == "__main__":
    main()