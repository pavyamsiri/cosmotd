import os


if __name__ == "__main__":
    min_value = 1
    max_value = 3
    todo_list = []
    for n in range(min_value, max_value + 1):
        for n_prime in range(min_value, max_value + 1):
            for m in range(min_value, max_value + 1):
                for m_prime in range(min_value, max_value + 1):
                    if (n * m_prime) == (n_prime * m):
                        continue
                    else:
                        # Check that the folder exists
                        identifier_name = f"N{n}{n_prime}{m}{m_prime}"
                        if not os.path.isdir(
                            f"../build/data/100_trials_ca_{identifier_name}"
                        ):
                            todo_list.append(identifier_name)

    if len(todo_list) == 0:
        print("All complete!")
    else:
        print("Missing the following:")
        for identifier_name in todo_list:
            print(identifier_name)
