if __name__ == "__main__":
    min_value = 1
    max_value = 3
    for n in range(min_value, max_value + 1):
        for n_prime in range(min_value, max_value + 1):
            for m in range(min_value, max_value + 1):
                for m_prime in range(min_value, max_value + 1):
                    if (n * m_prime) == (n_prime * m):
                        continue
                    else:
                        print(f"N{n}{n_prime}{m}{m_prime}")
