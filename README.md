# SoalShiftSISOP20_modul3_A11
## Kevin Angga Wijaya - 05111840000024
## Julius - 05111840000082
![523985](https://user-images.githubusercontent.com/60419316/78343204-b04b8300-75c4-11ea-9141-c19820f7f6ef.jpg)

## Soal 1
**Poke\*ZONE** adalah sebuah game berbasis text terminal mirip dengan Pokemon GO.
Ketentuan permainan sebagai berikut:
 1. Menggunakan IPC-shared memory, thread, fork-exec.
 2. Kalian bebas berkreasi dengan game ini asal tidak konflik dengan requirements yang ada. (Contoh: memberi nama trainer, memberi notifikasi kalau barang di shop sudah penuh, dan lain-lain).
 3. Terdapat 2 code yaitu soal2_traizone.c dan soal2_pokezone.c.
 4. **soal2_traizone.c** mengandung fitur:
   i. Normal Mode (3 Menu)
     - Cari Pokemon
        1. Jika diaktifkan maka setiap 10 detik akan memiliki 60% chance untuk menemui pokemon bertipe normal, rare, legendary dengan encounter rate sesuai pada tabel di bawah (Gunakan Thread).
        2. Cari pokemon hanya mengatur dia menemukan pokemon atau tidak. Untuk tipe pokemon dan pokemon tersebut shiny atau tidak diatur oleh soal2_pokezone.c.
        3. Opsi “Cari Pokemon” akan berubah jadi “Berhenti Mencari” ketika state mencari pokemon aktif.
        4. State mencari pokemon hanya bisa dihentikan ketika pokemon sudah ditemukan atau trainer memilih untuk berhenti pada menu.
        5. Jika menemui pokemon maka akan masuk ke Capture Mode.
        6. Untuk mempermudah boleh menambah menu go to capture mode untuk berpindah dari mode normal ke mode capture setelah menemukan pokemon dari thread Cari Pokemon.
