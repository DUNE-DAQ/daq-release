/***********************************************************************
 *
 * FILE:
 *       upsugo.h
 * 
 * DESCRIPTION: 
 *       Prototypes for upsugo
 *
 * AUTHORS:
 *       Eileen Berman
 *       David Fagan
 *       Lars Rasmussen
 *
 *       Fermilab Computing Division
 *       Batavia, Il 60510, U.S.A.                                              * MODIFICATIONS:
 *       06 Oct 1997, DjF First 
 *
 ***********************************************************************/

#ifndef _UPSGET_H_
#define _UPSGET_H_

#define UPSSHELL "UPS_SHELL"

char *upsget_translation( const t_upstyp_matched_instance * const minstance,
			  const t_upstyp_db * const db_info_ptr,
			  const t_upsugo_command * const command_line,
			  char * const oldstr );
char *upsget_translation_env( char * const oldstr );
char *upsget_prod_dir(const t_upstyp_db * const,
                      const t_upstyp_matched_instance * const ,
                      const t_upsugo_command * const );
char *upsget_ups_dir(const t_upstyp_db * const,
                      const t_upstyp_matched_instance * const ,
                      const t_upsugo_command * const );
char *upsget_source(const t_upstyp_db * const,
		    const t_upstyp_matched_instance * const ,
		    const t_upsugo_command * const );
char *upsget_compile(const t_upstyp_db * const,
                     const t_upstyp_matched_instance * const ,
                     const t_upsugo_command * const );
char *upsget_product(const t_upstyp_db * const,
                     const t_upstyp_matched_instance * const ,
                     const t_upsugo_command * const );
char *upsget_product_uc(const t_upstyp_db * const,
			const t_upstyp_matched_instance * const ,
			const t_upsugo_command * const );
char *upsget_version(const t_upstyp_db * const,
                     const t_upstyp_matched_instance * const ,
                     const t_upsugo_command * const );
char *upsget_flavor(const t_upstyp_db * const,
                    const t_upstyp_matched_instance * const ,
                    const t_upsugo_command * const );
char *upsget_qualifiers(const t_upstyp_db * const,
                        const t_upstyp_matched_instance * const ,
                        const t_upsugo_command * const );
char *upsget_reqqualifiers(const t_upstyp_db * const,
                        const t_upstyp_matched_instance * const ,
                        const t_upsugo_command * const );
char *upsget_reqchain(const t_upstyp_db * const,
                        const t_upstyp_matched_instance * const ,
                        const t_upsugo_command * const );
char *upsget_shell(const t_upstyp_db * const,
                   const t_upstyp_matched_instance * const ,
                   const t_upsugo_command * const );
char *upsget_source(const t_upstyp_db * const,
                   const t_upstyp_matched_instance * const ,
                   const t_upsugo_command * const );
char *upsget_verbose(const t_upstyp_db * const,
                     const t_upstyp_matched_instance * const ,
                     const t_upsugo_command * const );
char *upsget_options(const t_upstyp_db * const,
                     const t_upstyp_matched_instance * const ,
                     const t_upsugo_command * const );
char *upsget_this_db(const t_upstyp_db * const,
                      const t_upstyp_matched_instance * const ,
                      const t_upsugo_command * const );
char *upsget_OS_flavor(const t_upstyp_db * const,
                       const t_upstyp_matched_instance * const ,
                       const t_upsugo_command * const );
char *upsget_extended(const t_upstyp_db * const,
                      const t_upstyp_matched_instance * const ,
                      const t_upsugo_command * const );
void upsget_remall(const FILE * const,
                    const t_upsugo_command * const,
		    const char * const prefix);
void upsget_allout(const FILE * const,
                    const t_upstyp_db * const,
                    const t_upstyp_matched_instance * const,
                    const t_upsugo_command * const,
		    const char * const prefix);
char *upsget_envstr(const t_upstyp_db * const,
                    const t_upstyp_matched_instance * const ,
                    const t_upsugo_command * const );
void upsget_envout(const FILE * const,
                    const t_upstyp_db * const,
                    const t_upstyp_matched_instance * const,
                    const t_upsugo_command * const);
char *upsget_origin(const t_upstyp_db * const,
                    const t_upstyp_matched_instance * const ,
                    const t_upsugo_command * const );
t_upstyp_product *upsget_chain_file(const char * const,
                                    const char * const,
                                    const char * const,
                                    char ** const);
t_upstyp_product *upsget_version_file (const char * const,
                                       const char * const,
                                       const char * const,
                                       char ** const);
char *upsget_database(const t_upstyp_db * const,
                      const t_upstyp_matched_instance * const ,
                      const t_upsugo_command * const );
char *upsget_tilde_dir(char * const );
int upsget_key(const t_upstyp_instance * const);
char *upsget_man_source_dir( const t_upstyp_matched_instance * const ,
                             const t_upstyp_db * const ,
                             const t_upsugo_command * const );
char *upsget_catman_source_dir( const t_upstyp_matched_instance * const ,
                                const t_upstyp_db * const ,
                                const t_upsugo_command * const );
char *upsget_info_source_dir( const t_upstyp_matched_instance * const ,
                              const t_upstyp_db * const ,
                              const t_upsugo_command * const );
char *upsget_table_file( const char * const ,
                         const char * const ,
                         const char * const ,
                         const char * const ,
                         const char * const ,
                         const t_upstyp_db * const ,
                         const int );
char *upsget_archive_file(const t_upstyp_db * const ,
                          const t_upstyp_matched_instance * const ,
                          const t_upsugo_command * const ,
                          const int );
#endif /* _UPSGET_H_ */
