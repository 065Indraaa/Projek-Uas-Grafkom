#define _CRT_SECURE_NO_WARNINGS 

#include <GL/glut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stdio.h> 

// --- VARIABEL KAMERA & POSISI ---
float x = 0.0f, z = 15.0f;
float y = 1.8f;
float lx = 0.0f, lz = -1.0f;
float ly = 0.0f;
float angleYaw = 0.0f;
float anglePitch = 0.0f;

float moveSpeed = 0.5f;
float turnSpeed = 0.04f;

// --- VARIABEL PENCAYAHAN & LANGIT ---
float sunAngle = 45.0f;      // derajat, 0-360 (posisi matahari/bulan)
GLfloat skyColor[4] = { 0.53f, 0.81f, 0.92f, 1.0f }; // default langit siang

// default material specular (untuk restore)
GLfloat g_defaultSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat g_defaultShininess[] = { 64.0f };

// ---------- FUNGSI TEXT & HUD ----------
void drawText3D(const char* text, float scale, float lineWidth) {
    glPushMatrix();
    glScalef(scale, scale, scale);
    glLineWidth(lineWidth);
    for (int i = 0; i < (int)strlen(text); i++) {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, text[i]);
    }
    glPopMatrix();
}

void renderBitmapString(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}

// --- UPDATE PENCAYAHAN (MATAHARI & BULAN) ---
void updateLighting() {
    float rad = sunAngle * (float)M_PI / 180.0f;

    // Orbit matahari
    float sunX = 80.0f * cosf(rad);
    float sunY = 55.0f * sinf(rad);
    float sunZ = 40.0f * sinf(rad * 0.5f);

    // Orbit bulan (berlawanan)
    float moonX = -sunX;
    float moonY = -sunY * 0.9f;
    float moonZ = -sunZ;

    // Intensitas siang berdasarkan tinggi matahari
    float daylight = (sunY > 0.0f ? sunY / 55.0f : 0.0f);
    if (daylight < 0.0f) daylight = 0.0f;
    if (daylight > 1.0f) daylight = 1.0f;

    float nightFactor = 1.0f - daylight;

    // ================================
    // CAHAYA MATAHARI (GL_LIGHT0)
    // ================================
    GLfloat sunPos[4] = { sunX, sunY, sunZ, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);

    GLfloat sunAmbient[4] = { 0.20f * daylight, 0.20f * daylight, 0.20f * daylight, 1.0f };
    GLfloat sunDiffuse[4] = { 1.00f * daylight, 0.95f * daylight, 0.85f * daylight, 1.0f };
    // specular diperkuat untuk pantulan yang jelas
    GLfloat sunSpecular[4] = { 2.0f * daylight, 2.0f * daylight, 2.0f * daylight, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);

    // ================================
    // CAHAYA BULAN (GL_LIGHT1)
    // ================================
    GLfloat moonPos[4] = { moonX, moonY, moonZ, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, moonPos);

    GLfloat moonAmbient[4] = { 0.05f * nightFactor, 0.05f * nightFactor, 0.12f * nightFactor, 1.0f };
    GLfloat moonDiffuse[4] = { 0.30f * nightFactor, 0.35f * nightFactor, 0.50f * nightFactor, 1.0f };
    GLfloat moonSpecular[4] = { 0.60f * nightFactor, 0.65f * nightFactor, 0.80f * nightFactor, 1.0f };

    glLightfv(GL_LIGHT1, GL_AMBIENT, moonAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, moonDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, moonSpecular);

    // ================================
    // WARNA LANGIT
    // ================================
    skyColor[0] = 0.05f + daylight * (0.53f - 0.05f);
    skyColor[1] = 0.05f + daylight * (0.81f - 0.05f);
    skyColor[2] = 0.10f + daylight * (0.92f - 0.10f);
    skyColor[3] = 1.0f;
}

// --- HUD REAL-TIME ---
void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0.0, 1280.0, 0.0, 720.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    char bufferCoord[128];
    sprintf(bufferCoord, "XYZ: %.2f / %.2f / %.2f", x, y, z);

    glColor3f(0.0f, 0.0f, 0.0f);
    renderBitmapString(21, 689, GLUT_BITMAP_HELVETICA_18, bufferCoord);
    glColor3f(1.0f, 1.0f, 0.0f);
    renderBitmapString(20, 690, GLUT_BITMAP_HELVETICA_18, bufferCoord);

    glColor3f(0.0f, 0.0f, 0.0f);
    renderBitmapString(20, 670, GLUT_BITMAP_HELVETICA_12,
        "[WASD]: Jalan | [SPASI]: Terbang | [C]: Turun");

    glEnable(GL_DEPTH_TEST);
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
}

// --- RENDER MATAHARI & BULAN (visual) ---
void drawSunMoon() {
    glDisable(GL_LIGHTING);

    float rad = sunAngle * (float)M_PI / 180.0f;

    float sunX = 80.0f * cosf(rad);
    float sunY = 55.0f * sinf(rad);
    float sunZ = 40.0f * sinf(rad * 0.5f);

    float moonX = -sunX;
    float moonY = -sunY * 0.9f;
    float moonZ = -sunZ;

    // Jika matahari di atas horizon -> tampilkan matahari
    if (sunY > 0.0f) {
        glPushMatrix();
        glColor3f(1.0f, 0.85f, 0.15f);
        glTranslatef(sunX, sunY, sunZ);
        glutSolidSphere(3.0f, 32, 32);
        glPopMatrix();
    }
    // Jika matahari di bawah horizon -> tampilkan bulan (kalau di atas horizon)
    else if (moonY > 0.0f) {
        glPushMatrix();
        glColor3f(0.75f, 0.82f, 1.0f);
        glTranslatef(moonX, moonY, moonZ);
        glutSolidSphere(2.5f, 32, 32);
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
}
// --- OBJEK: GEDUNG GRAHA UNESA (ASLI) ---
void drawGedungUnesa() {
    glPushMatrix();
    glTranslatef(48.0f, 0.0f, -25.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    // 1. Badan Utama
    glPushMatrix();
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(0.0f, 7.5f, 2.5f);
    glScalef(30.0f, 15.0f, 19.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 2. Kaca Depan Dasar (Background Biru)
    // Beri material kaca lebih glossy
    GLfloat glassSpec[] = { 0.9f, 0.95f, 1.0f, 1.0f };
    GLfloat glassShine[] = { 96.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, glassSpec);
    glMaterialfv(GL_FRONT, GL_SHININESS, glassShine);

    glPushMatrix();
    glColor3f(0.3f, 0.7f, 0.9f);
    glTranslatef(0.0f, 7.5f, -7.5f);
    glScalef(28.0f, 12.0f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // reset material ke default
    glMaterialfv(GL_FRONT, GL_SPECULAR, g_defaultSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, g_defaultShininess);

    // 3. Pilar Vertikal
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = -4; i <= 4; i++) {
        glPushMatrix();
        float posX = i * 3.5f;
        glTranslatef(posX, 7.5f, -8.2f);
        glScalef(0.8f, 14.0f, 0.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Tembok timbul tengah
    glPushMatrix();
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(0.0f, 7.5f, -9.0f);
    glScalef(8.0f, 15.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix(); // Aksen atas
    glColor3f(0.98f, 0.98f, 0.98f);
    glTranslatef(0.0f, 13.0f, -10.1f);
    glScalef(6.0f, 2.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Jendela tengah (3 lantai)
    glColor3f(0.0f, 0.3f, 0.6f);
    glPushMatrix(); glTranslatef(0.0f, 10.5f, -10.1f); glScalef(6.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 7.5f, -10.1f); glScalef(6.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 4.5f, -10.1f); glScalef(6.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();

    // Pintu
    glPushMatrix(); glColor3f(0.7f, 0.7f, 0.7f); glTranslatef(0.0f, 1.5f, -10.1f);  glScalef(5.0f, 3.0f, 0.2f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glColor3f(0.1f, 0.1f, 0.2f); glTranslatef(-1.2f, 1.5f, -10.15f); glScalef(2.2f, 2.8f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix();                                    glTranslatef(1.2f, 1.5f, -10.15f); glScalef(2.2f, 2.8f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(1.0f, 0.8f, 0.0f);
    glPushMatrix(); glTranslatef(-0.3f, 1.5f, -10.2f); glutSolidSphere(0.15f, 10, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(0.3f, 1.5f, -10.2f); glutSolidSphere(0.15f, 10, 10); glPopMatrix();

    // Sisi kiri
    glPushMatrix();
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(-12.0f, 7.5f, -9.0f);
    glScalef(4.0f, 15.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.0f, 0.3f, 0.6f);
    glPushMatrix(); glTranslatef(-12.0f, 10.5f, -10.1f); glScalef(3.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-12.0f, 7.5f, -10.1f); glScalef(3.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-12.0f, 4.5f, -10.1f); glScalef(3.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();

    // Sisi kanan
    glPushMatrix();
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(12.0f, 7.5f, -9.0f);
    glScalef(4.0f, 15.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.0f, 0.3f, 0.6f);
    glPushMatrix(); glTranslatef(12.0f, 10.5f, -10.1f); glScalef(3.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(12.0f, 7.5f, -10.1f); glScalef(3.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(12.0f, 4.5f, -10.1f); glScalef(3.0f, 2.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();

    // Jendela samping sayap
    glPushMatrix();
    glColor3f(0.0f, 0.0f, 0.8f);
    glTranslatef(-15.1f, 6.0f, 2.5f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(4.0f, 5.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.0f, 0.0f, 0.8f);
    glTranslatef(15.1f, 6.0f, 2.5f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(4.0f, 5.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Atap & selimut
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, 14.5f, 2.5f);
    glScalef(1.7f, 0.5f, 1.2f);
    glutSolidSphere(10.0f, 40, 40);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(0.0f, 16.0f, 6.0f);
    glScalef(0.9f, 0.4f, 1.6f);
    glutSolidSphere(10.0f, 40, 40);
    glPopMatrix();

    // Judul
    glPushMatrix();
    glColor3f(0.1f, 0.1f, 0.1f);
    glTranslatef(5.0f, 16.2f, -10.2f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    drawText3D("GRAHA UNESA", 0.008f, 4.0f);
    glPopMatrix();

    glPopMatrix();
}

// --- LINGKUNGAN ---
void drawJalan() {
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-3.0f, 0.0f, 5.0f);  glVertex3f(3.0f, 0.0f, 5.0f);
    glVertex3f(3.0f, 0.0f, -25.0f); glVertex3f(-3.0f, 0.0f, -25.0f);
    glVertex3f(-30.0f, 0.0f, -20.0f); glVertex3f(30.0f, 0.0f, -20.0f);
    glVertex3f(30.0f, 0.0f, -30.0f);  glVertex3f(-30.0f, 0.0f, -30.0f);
    glEnd();
}

void drawGapura() {
    glPushMatrix(); glColor3f(0.8f, 0.2f, 0.2f); glTranslatef(-3.5f, 2.5f, 0.0f); glScalef(1.5f, 5.0f, 1.5f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glColor3f(0.8f, 0.2f, 0.2f); glTranslatef(3.5f, 2.5f, 0.0f);  glScalef(1.5f, 5.0f, 1.5f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glColor3f(0.9f, 0.7f, 0.0f); glTranslatef(0.0f, 5.5f, 0.0f);  glScalef(9.0f, 1.5f, 2.0f); glutSolidCube(1.0f); glPopMatrix();
}

void drawTanah() {
    glColor3f(0.1f, 0.6f, 0.1f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-100.0f, -0.1f, 100.0f);  glVertex3f(100.0f, -0.1f, 100.0f);
    glVertex3f(100.0f, -0.1f, -100.0f); glVertex3f(-100.0f, -0.1f, -100.0f);
    glEnd();
}

// --- OBJEK: GEDUNG PSIKOLOGI UNESA ---
void drawGedungPsikologi() {
    glPushMatrix();

    // Posisi global gedung psikologi
    glTranslatef(-30.39f, 0.20f, -23.66f);

    // ================= PARAMETER UTAMA =================
    const int   jmlLantai = 7;
    const float tinggiLantai = 2.2f;
    const float tinggiTotal = jmlLantai * tinggiLantai + 1.0f;
    const float baseY = 0.0f;

    // Fasad lengkung kanan -> pakai arc sederhana
    const float radiusLengkung = 4.8f;
    const float centerX = 2.5f;
    const float centerZ = 0.0f;
    const float thetaMulai = -70.0f;
    const float thetaAkhir = 70.0f;
    const float thetaStep = 10.0f;

    // Warna
    const GLfloat kacaR = 0.0f, kacaG = 0.58f, kacaB = 0.46f;   // hijau toska
    const GLfloat tiangR = 0.20f, tiangG = 0.20f, tiangB = 0.20f;  // abu gelap

    // ===================================================
    // 1. PODIUM & SELASAR BAWAH
    // ===================================================
    glPushMatrix();
    glColor3f(0.86f, 0.86f, 0.86f);
    glTranslatef(0.0f, baseY - 0.2f, 0.0f);
    glScalef(34.0f, 0.8f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.92f, 0.92f, 0.92f);
    glTranslatef(0.0f, baseY + 0.8f, 4.2f);
    glScalef(28.0f, 2.0f, 4.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.97f, 0.97f, 0.97f);
    glTranslatef(-1.0f, baseY + 3.0f, -0.5f);
    glScalef(26.0f, 0.6f, 9.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.93f, 0.93f, 0.93f);
    for (int i = -5; i <= 5; ++i) {
        glPushMatrix();
        glTranslatef(-8.0f + i * 2.6f, baseY + 1.7f, -3.5f);
        glScalef(0.9f, 3.4f, 0.9f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    glColor3f(0.94f, 0.94f, 0.94f);
    for (int i = -5; i <= 5; ++i) {
        glPushMatrix();
        glTranslatef(-7.0f + i * 2.6f, baseY + 1.7f, 6.3f);
        glScalef(0.4f, 3.4f, 0.4f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // ===================================================
    // 2. BLOK KIRI (GEDUNG LURUS)
    // ===================================================
    glPushMatrix();
    glColor3f(0.97f, 0.97f, 0.97f);
    glTranslatef(-9.0f, baseY + tinggiTotal * 0.5f, 0.0f);
    glScalef(16.0f, tinggiTotal, 9.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(tiangR, tiangG, tiangB);
    for (int k = 0; k < 7; ++k) {
        float xPanel = -7.5f + k * 2.1f;
        glPushMatrix();
        glTranslatef(xPanel, baseY + tinggiTotal * 0.5f, 4.5f);
        glScalef(0.28f, tinggiTotal, 0.2f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Strip jendela depan (bisa sedikit glossy, tapi pakai default saja)
    glColor3f(kacaR, kacaG, kacaB);
    for (int lantai = 0; lantai < 6; ++lantai) {
        float y = baseY + 2.0f + lantai * tinggiLantai;
        float scaleX = (lantai == 0) ? 11.5f : 12.5f;

        glPushMatrix();
        glTranslatef(-9.0f, y, 4.0f);
        glScalef(scaleX, 0.7f, 0.15f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    for (int lantai = 0; lantai < 6; ++lantai) {
        float y = baseY + 2.0f + lantai * tinggiLantai;
        float scaleX = (lantai == 0) ? 11.5f : 12.5f;

        glPushMatrix();
        glTranslatef(-9.0f, y, -4.0f);
        glScalef(scaleX, 0.7f, 0.15f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // ===================================================
    // 3. BLOK KANAN LENGKUNG
    // ===================================================
    // dinding putih melengkung
    for (float theta = thetaMulai; theta <= thetaAkhir; theta += thetaStep) {
        float rad = theta * (float)M_PI / 180.0f;
        float cx = centerX + radiusLengkung * cosf(rad);
        float cz = centerZ + radiusLengkung * sinf(rad);

        glPushMatrix();
        glColor3f(0.98f, 0.98f, 0.98f);
        glTranslatef(cx, baseY + tinggiTotal * 0.5f, cz);
        glRotatef(-theta, 0.0f, 1.0f, 0.0f);
        glScalef(1.4f, tinggiTotal, 0.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // kaca melengkung – beri specular kaca
    GLfloat glassSpec2[] = { 0.9f, 0.95f, 1.0f, 1.0f };
    GLfloat glassShine2[] = { 96.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, glassSpec2);
    glMaterialfv(GL_FRONT, GL_SHININESS, glassShine2);

    const int jmlStripLengkung = 6;
    for (int lantai = 0; lantai < jmlStripLengkung; ++lantai) {
        float y = baseY + 2.4f + lantai * tinggiLantai;

        for (float theta = thetaMulai + 4.0f; theta <= thetaAkhir - 4.0f; theta += thetaStep) {
            float rad = theta * (float)M_PI / 180.0f;
            float cx = centerX + (radiusLengkung + 0.10f) * cosf(rad);
            float cz = centerZ + (radiusLengkung + 0.10f) * sinf(rad);

            float scaleX = (lantai == 0) ? 1.0f : 1.15f;

            glPushMatrix();
            glColor3f(kacaR, kacaG, kacaB);
            glTranslatef(cx, y, cz);
            glRotatef(-theta, 0.0f, 1.0f, 0.0f);
            glScalef(scaleX, 0.65f, 0.12f);
            glutSolidCube(1.0f);
            glPopMatrix();

            glPushMatrix();
            glColor3f(tiangR, tiangG, tiangB);
            glTranslatef(cx, y, cz + 0.03f);
            glRotatef(-theta, 0.0f, 1.0f, 0.0f);
            glScalef(0.08f, 0.75f, 0.08f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    // reset material kaca ke default
    glMaterialfv(GL_FRONT, GL_SPECULAR, g_defaultSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, g_defaultShininess);

    // ===================================================
    // 4. PINTU MASUK & KANOPI
    // ===================================================
    float pintuX = centerX + radiusLengkung + 0.45f;

    glColor3f(0.92f, 0.92f, 0.92f);
    for (int i = -2; i <= 2; ++i) {
        if (i == 0) continue;
        glPushMatrix();
        glTranslatef(pintuX - 1.6f, baseY + 1.9f, i * 1.0f);
        glScalef(0.45f, 3.8f, 0.45f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    glPushMatrix();
    glColor3f(0.96f, 0.96f, 0.96f);
    glTranslatef(pintuX, baseY + 3.8f, 0.0f);
    glScalef(7.0f, 0.5f, 4.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.86f, 0.86f, 0.86f);
    glTranslatef(pintuX + 0.15f, baseY + 2.3f, 0.0f);
    glScalef(0.5f, 3.0f, 2.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.08f, 0.20f, 0.25f);
    glTranslatef(pintuX + 0.20f, baseY + 2.3f, -0.7f);
    glScalef(0.18f, 2.5f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.08f, 0.20f, 0.25f);
    glTranslatef(pintuX + 0.20f, baseY + 2.3f, 0.7f);
    glScalef(0.18f, 2.5f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.98f, 0.98f, 0.98f);
    glTranslatef(pintuX - 0.2f, baseY + 4.9f, 0.0f);
    glScalef(4.6f, 0.6f, 2.6f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ===================================================
    // 5. LOGO & ATAP
    // ===================================================
    glPushMatrix();
    glColor3f(1.0f, 0.9f, 0.3f);
    glTranslatef(centerX + radiusLengkung * 0.85f,
        baseY + tinggiTotal + 0.8f, 0.2f);
    glScalef(1.4f, 1.4f, 0.25f);
    glutSolidSphere(1.0f, 24, 24);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.80f, 0.75f, 0.70f);
    glTranslatef(-1.0f, baseY + tinggiTotal + 1.3f, 0.0f);
    glRotatef(6.0f, 0.0f, 0.0f, 1.0f);
    glScalef(20.0f, 0.7f, 10.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ===================================================
    // 6. BUNDARAN / AIR MANCUR DEPAN
    // ===================================================
    glPushMatrix();
    glTranslatef(pintuX + 7.8f, 0.0f, 0.0f);

    glPushMatrix();
    glColor3f(0.25f, 0.25f, 0.25f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.27f, 3.5f, 30, 40);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.35f, 0.35f, 0.35f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.20f, 2.3f, 30, 40);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.1f, 0.4f, 0.6f);
    glTranslatef(0.0f, 0.48f, 0.0f);
    glutSolidSphere(0.8f, 20, 20);
    glPopMatrix();

    glPopMatrix(); // bundaran
    glPopMatrix(); // gedung psikologi
}
// --- RENDER SCENE ---
void renderScene(void) {
    // update posisi & warna cahaya
    updateLighting();

    glClearColor(skyColor[0], skyColor[1], skyColor[2], skyColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(x, y, z, x + lx, y + ly, z + lz, 0.0f, 1.0f, 0.0f);

    drawTanah();
    drawJalan();
    drawGapura();
    drawGedungUnesa();
    drawGedungPsikologi();
    drawSunMoon();  // matahari/bulan di langit
    drawHUD();

    glutSwapBuffers();
}

void changeSize(int w, int h) {
    if (h == 0) h = 1;
    float ratio = w * 1.0f / h;
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glViewport(0, 0, w, h);
    gluPerspective(45.0f, ratio, 0.1f, 300.0f);
    glMatrixMode(GL_MODELVIEW);
}

// --- IDLE: SIANG-MALAM OTOMATIS ---
void idleFunc() {
    // matahari & bulan selalu berputar otomatis
    sunAngle += 0.02f;
    if (sunAngle >= 360.0f) sunAngle -= 360.0f;

    glutPostRedisplay();
}

// --- INPUT USER ---
void processNormalKeys(unsigned char key, int xx, int yy) {
    float fraction = moveSpeed;
    switch (key) {
    case 'w': case 'W': x += lx * fraction; z += lz * fraction; y += ly * fraction; break;
    case 's': case 'S': x -= lx * fraction; z -= lz * fraction; y -= ly * fraction; break;
    case 'a': case 'A': x += lz * fraction; z -= lx * fraction; break;
    case 'd': case 'D': x -= lz * fraction; z += lx * fraction; break;
    case ' ':          y += fraction; break;
    case 'c': case 'C': y -= fraction; if (y < 0.5f) y = 0.5f; break;
    case 27: exit(0); break;
    }
}

void processSpecialKeys(int key, int xx, int yy) {
    switch (key) {
    case GLUT_KEY_LEFT:  angleYaw -= turnSpeed; lx = sin(angleYaw); lz = -cos(angleYaw); break;
    case GLUT_KEY_RIGHT: angleYaw += turnSpeed; lx = sin(angleYaw); lz = -cos(angleYaw); break;
    case GLUT_KEY_UP:    if (anglePitch < 1.0f) { anglePitch += turnSpeed; ly = sin(anglePitch); } break;
    case GLUT_KEY_DOWN:  if (anglePitch > -1.0f) { anglePitch -= turnSpeed; ly = sin(anglePitch); } break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("UNESA WORLD - GEDUNG + MATAHARI/BULAN OTOMATIS + PANTULAN");

    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutIdleFunc(idleFunc);
    glutKeyboardFunc(processNormalKeys);
    glutSpecialFunc(processSpecialKeys);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    // --- ENABLE LIGHTING ---
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Matahari
    glEnable(GL_LIGHT1); // Bulan
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE); // normal diperbaiki setelah scaling

    // MATERIAL SPECULAR AGAR ADA PANTULAN CAHAYA
    glMaterialfv(GL_FRONT, GL_SPECULAR, g_defaultSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, g_defaultShininess);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    glutMainLoop();
    return 0;
}
