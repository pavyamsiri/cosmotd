if __name__ == "__main__":
    models = []
    min_value = 1
    max_value = 3
    count = 0
    for n in range(min_value, max_value + 1):
        for n_prime in range(min_value, max_value + 1):
            for m in range(min_value, max_value + 1):
                for m_prime in range(min_value, max_value + 1):
                    if (n * m_prime) == (n_prime * m):
                        continue
                    else:
                        reverse_identifier = f"N{n_prime}{n}{m_prime}{m}"
                        if reverse_identifier in models:
                            continue
                        else:
                            identifier = f"N{n}{n_prime}{m}{m_prime}"
                            models.append(identifier)
                            count += 1
                            print(f"{count}: {identifier}")
