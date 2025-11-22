#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

void connectDB() {
    conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "mysql_init() gagal\n");
        exit(1);
    }

    if (!mysql_real_connect(
            conn,
            "localhost", "root", "", "siakad",
            3320,
            "/Applications/XAMPP/xamppfiles/var/mysql/mysql.sock",
            0)) {
        fprintf(stderr, "Koneksi gagal: %s\n", mysql_error(conn));
        exit(1);
    }
}

void tambahMahasiswa() {
    char nim[20], nama[100];
    printf("Masukkan NIM Mahasiswa: ");
    scanf("%s", nim);
    printf("Masukkan Nama Mahasiswa: ");
    scanf(" %[^\n]", nama);

    char query[200];
    sprintf(query, "INSERT INTO mahasiswa (nim, nama) VALUES('%s','%s')", nim, nama);

    if (mysql_query(conn, query) == 0)
        printf("✅ Mahasiswa berhasil ditambahkan!\n\n");
    else
        fprintf(stderr, "❌ Gagal tambah mahasiswa: %s\n\n", mysql_error(conn));
}

void tambahDosen() {
    char nip[20], nama[100], matkul[100];
    printf("Masukkan NIP Dosen: ");
    scanf("%s", nip);
    printf("Masukkan Nama Dosen: ");
    scanf(" %[^\n]", nama);
    printf("Masukkan Mata Kuliah: ");
    scanf(" %[^\n]", matkul);

    char query[300];
    sprintf(query, "INSERT INTO dosen (nip, nama, mata_kuliah) VALUES('%s','%s','%s')", nip, nama, matkul);

    if (mysql_query(conn, query) == 0)
        printf("✅ Dosen berhasil ditambahkan!\n\n");
    else
        fprintf(stderr, "❌ Gagal tambah dosen: %s\n\n", mysql_error(conn));
}

void tambahKRS() {
    char nim[20], matkul[100], nama_dosen[100], nip_dosen[20];

    printf("Masukkan NIM Mahasiswa: ");
    scanf("%s", nim);

    // Validasi mahasiswa terdaftar
    char cek_mhs[100];
    sprintf(cek_mhs, "SELECT 1 FROM mahasiswa WHERE nim='%s'", nim);
    if (mysql_query(conn, cek_mhs) != 0) {
        fprintf(stderr, "❌ Gagal cek mahasiswa: %s\n", mysql_error(conn));
        return;
    }
    MYSQL_RES *res_mhs = mysql_store_result(conn);
    if (!mysql_fetch_row(res_mhs)) {
        printf("⚠️ NIM '%s' belum terdaftar.\n\n", nim);
        mysql_free_result(res_mhs);
        return;
    }
    mysql_free_result(res_mhs);

    printf("Masukkan Mata Kuliah: ");
    scanf(" %[^\n]", matkul);
    printf("Masukkan Nama Dosen: ");
    scanf(" %[^\n]", nama_dosen);

    // Validasi dosen mengajar matkul
    char query_nip[300];
    sprintf(query_nip,
            "SELECT nip FROM dosen WHERE nama='%s' AND mata_kuliah='%s'",
            nama_dosen, matkul);

    if (mysql_query(conn, query_nip) != 0) {
        fprintf(stderr, "❌ Gagal cek dosen & matkul: %s\n", mysql_error(conn));
        return;
    }

    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);

    if (!row) {
        printf("⚠️ Dosen '%s' tidak mengajar matkul '%s'.\n\n", nama_dosen, matkul);
        mysql_free_result(res);
        return;
    }

    strcpy(nip_dosen, row[0]);
    mysql_free_result(res);

    // Cek duplikasi KRS
    char cek_duplikat[300];
    sprintf(cek_duplikat,
            "SELECT 1 FROM krs WHERE nim='%s' AND mata_kuliah='%s'",
            nim, matkul);

    if (mysql_query(conn, cek_duplikat) != 0) {
        fprintf(stderr, "❌ Gagal cek duplikasi KRS: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *res_dup = mysql_store_result(conn);
    if (mysql_fetch_row(res_dup)) {
        printf("⚠️ Mahasiswa %s sudah mengambil matkul '%s'.\n\n", nim, matkul);
        mysql_free_result(res_dup);
        return;
    }
    mysql_free_result(res_dup);

    // Insert KRS
    char query_insert[350];
    sprintf(query_insert,
            "INSERT INTO krs (nim, mata_kuliah, nip_dosen) VALUES('%s','%s','%s')",
            nim, matkul, nip_dosen);

    if (mysql_query(conn, query_insert) == 0)
        printf("✅ KRS berhasil ditambahkan: %s — %s (Dosen: %s)\n\n",
               nim, matkul, nama_dosen);
    else
        fprintf(stderr, "❌ Gagal tambah KRS: %s\n\n", mysql_error(conn));
}

void lihatKRS() {
    char nim[20];
    printf("Masukkan NIM Mahasiswa: ");
    scanf("%s", nim);

    char query[300];
    sprintf(query,
        "SELECT k.mata_kuliah, d.nama "
        "FROM krs k JOIN dosen d ON k.nip_dosen = d.nip "
        "WHERE k.nim='%s'", nim);

    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "❌ Gagal ambil KRS: %s\n", mysql_error(conn));
        return;
    }

    res = mysql_store_result(conn);
    printf("\n=== KRS Mahasiswa %s ===\n", nim);
    while ((row = mysql_fetch_row(res))) {
        printf("Matkul: %s | Dosen: %s\n", row[0], row[1]);
    }
    mysql_free_result(res);
    printf("\n");
}

int main() {
    int pilihan;

    connectDB();

    do {
        printf("===== MENU SIAKAD =====\n");
        printf("1. Tambah Mahasiswa\n");
        printf("2. Tambah Dosen\n");
        printf("3. Tambah KRS Mahasiswa\n");
        printf("4. Lihat KRS Mahasiswa\n");
        printf("0. Keluar\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan);

        switch (pilihan) {
            case 1: tambahMahasiswa(); break;
            case 2: tambahDosen(); break;
            case 3: tambahKRS(); break;
            case 4: lihatKRS(); break;
        }

    } while (pilihan != 0);

    mysql_close(conn);
    return 0;
}
