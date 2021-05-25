#include <stdio.h>
#include "smtp.h"
#define MAIL_SERVER "smtp.gmail.com"
#define MAIL_PORT "587"
#define MAIL_CONNECTION_SECURITY SMTP_SECURITY_STARTTLS
#define MAIL_FLAGS (SMTP_DEBUG | \
                    SMTP_NO_CERT_VERIFY) /* Do not verify cert. */
#define MAIL_CAFILE NULL
#define MAIL_AUTH SMTP_AUTH_PLAIN
#define MAIL_USER "tuyenldjp@gmail.com"
#define MAIL_PASS "xybqcjezjkrzocjo"
#define MAIL_FROM "noreply@example.org"
#define MAIL_FROM_NAME "Tổ trưởng"
#define MAIL_SUBJECT "Phản hồi kiến nghị từ cấp trên 3"
// #define MAIL_BODY "Testttttttttttt 2"
#define MAIL_TO "ledinhtuyenpro@gmail.com"
#define MAIL_TO_NAME "Le Tuyen"
#define MAIL_CC "ninhthangnb@gmail.com"
#define MAIL_CC_NAME "Le Tuyen 2"
#define MAIL_BCC "le.tuyen.hust@gmail.com"
#define MAIL_BCC_NAME "Le Tuyen 3"
int main(void)
{
  const char *const email_body =
      "<html>\n"
      " <head><title>HTML Email</title></head>\n"
      " <body>\n"
      "  <h1>H1</h1>\n"
      "  <h2>H2</h1>\n"
      "  <h3>H3</h1>\n"
      "  <h4>H4</h1>\n"
      "  <h5>H5</h1>\n"
      "  <h6>H6</h1>\n"
      " </body>\n"
      "</html>\n";

  struct smtp *smtp;
  int rc;
  rc = smtp_open(MAIL_SERVER,
                 MAIL_PORT,
                 MAIL_CONNECTION_SECURITY,
                 MAIL_FLAGS,
                 MAIL_CAFILE,
                 &smtp);
  rc = smtp_auth(smtp,
                 MAIL_AUTH,
                 MAIL_USER,
                 MAIL_PASS);
  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_FROM,
                        MAIL_FROM,
                        MAIL_FROM_NAME);
  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_TO,
                        MAIL_TO,
                        MAIL_TO_NAME);
  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_TO,
                        "ninhthangnb@gmail.com",
                        "Le Tuyen 2");
  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_TO,
                        "ledinhtuyenpro@gmail.com",
                        "Le Tuyen 3");
  rc = smtp_header_add(smtp,
                       "Subject",
                       MAIL_SUBJECT);
  rc = smtp_header_add(smtp,
                       "Content-Type",
                       "text/html");
  rc = smtp_attachment_add_mem(smtp,
                               "test.txt",
                               "Test email attachment 2.",
                               -1);
  rc = smtp_mail(smtp,
                 email_body);
  rc = smtp_close(smtp);
  if (rc != SMTP_STATUS_OK)
  {
    fprintf(stderr, "smtp failed: %s\n", smtp_status_code_errstr(rc));
    return 1;
  }
  return 0;
}
