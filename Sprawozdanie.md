Kacper Majorkowski 151753

# Sprawozdanie sk2
## Zdalne zamykanie systemów operacyjnych

1. Opis projektu

Kompilacja: `make`

Serwer: C++ z wykorzystaniem biblioteki sqlite3 (dołączona do projektu)

Klient: Python 3.10

Aplikacja umożliwia połączenie wielu agentom (maszynom) do serwera.

Uprawnienia do akcji są autoryzowane poprzez logowanie jako klient (użytkownik).

Należy zarejestrować użytkownika `register user`

a następnie zalogować się `login user`

Wszystkie komendy zostały opisane w komendzie `help`

2. Opis komunikacji pomiędzy serwerem i klientem

Aplikacja jest zaimplementowana w architekturze klient-serwer.
Serwer obsługuje klientów korzystając z puli wątków (domyślnie 10 + główny).

Port serwera można skonfigurować w src/misc/const.hpp.

3. Podsumowanie

Serwer jest zaimplementowany obiektowo, główne operacje sieciowe wykonywane są w TCPConnection.

Główna trudność w projekcie wynikała z możliwości zamykania innych agentów niż aktualny.
W mojej implementacji zapisuje deskryptory poszczególnych połączeń (IP) w bazie.
Gdy zamykamy agenta z danym IP, następuje odczyt deskryptora z bazy, następnie tworzę TCPConnection dla danego deskryptora i wysyłam wiadomość zamykającą.
Aby uniknąć sytuacji w której dwa wątki piszą na ten sam deskryptor, owinąłem funkcję wysyłąjącą w mutex.
Dla lepszego skalowania można zmienić implementację na taką, która komunikuje się z odpowiednimi wątkami, a nie bezpośrednio z deskryptorami.
W takiej implementacji każdy wątek powinien mieć swoją kolejkę komunikatów do obsłużenia.
Uważam, że przy domyślnych 11 wątkach moja implementacja jest wystarczająca.

Klient opiera się na dwóch funkcjach - jednej obsłgującej wejście i drugiej, która obsługuje wyjście.

Tutaj problem sprawiało poprawne wyświetlanie komunikatów oraz "asynchroniczne" zamykanie.
