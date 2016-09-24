#include "emacs-module.h"
#include "musicbrainz5/Artist.h"
#include "musicbrainz5/HTTPFetch.h"
#include "musicbrainz5/Query.h"
#include "string.h"

/* `Message' `message` to Emacs. */
void debug(emacs_env* const env,
           const std::string& message)
{
  emacs_value Fmessage = env->intern(env, "message");
  emacs_value Qmessage = env->make_string(env,
                                          message.c_str(),
                                          message.length());
  env->funcall(env, Fmessage, 1, &Qmessage);
}

/* Calculate the length of `value`. */
intmax_t emacs_string_length(emacs_env* env,
                             emacs_value* value)
{
  emacs_value Flength = env->intern(env, "length");
  emacs_value Qlength = env->funcall(env, Flength, 1, value);
  return env->extract_integer(env, Qlength);
}

emacs_value execute_search(emacs_env *env,
                           ptrdiff_t nargs,
                           emacs_value args[],
                           void*)
  EMACS_NOEXCEPT
{
  // Elisp strings do not have a terminating NULL, therefore we need one more
  // byte here.
  intmax_t bufsize = emacs_string_length(env, args) + 1;

  char buffer[bufsize];

  if(!env->copy_string_contents(env, args[0], buffer, &bufsize))
    {
      return env->intern(env, "nil");
    }

  MusicBrainz5::CQuery::tParamMap params = MusicBrainz5::CQuery::tParamMap();
  params.insert(std::pair<std::string, std::string>("query", std::string(buffer)));

  MusicBrainz5::CQuery Query("emacsmodule-0.1");
  try
  {
    std::ostringstream stream;
    int results = 0;
    MusicBrainz5::CMetadata Metadata = Query.Query("artist",
                                                 "",
                                                 "",
                                                 params);
    if (Metadata.ArtistList() && Metadata.ArtistList())
      {
        MusicBrainz5::CArtistList *ArtistList = Metadata.ArtistList();
        results = ArtistList->NumItems();
        stream << "Found " << ArtistList->NumItems() << " release(s)" << std::endl;
        debug(env, stream.str());
        stream.clear();

        for (int count=0; count < ArtistList->NumItems(); count++)
          {
            MusicBrainz5::CArtist *Artist = ArtistList->Item(count);
            stream << "Basic artist: " << std::endl << *Artist << std::endl;
          }
      }

    stream << "Found " << results << " artists(s)" << std::endl;
    emacs_value retmessage = env->make_string(env,
                                              stream.str().c_str(),
                                              stream.str().length());
    return retmessage;
  }
  catch (MusicBrainz5::CExceptionBase& Error)
  {
    std::ostringstream errormessage;
    errormessage << "Connection Exception: '" << Error.what() << "'" << std::endl;
    errormessage << "LastResult: " << Query.LastResult() << std::endl;
    errormessage << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
    errormessage << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    debug(env, errormessage.str());

    emacs_value nil = env->intern(env, "nil");
    return nil;
  }
}

static void
bind_function (emacs_env *env, const char* name, emacs_value Sfun){
  /* Convert the strings to symbols by interning them */
  emacs_value Qfset = env->intern (env, "fset");
  emacs_value Qsym = env->intern (env, name);

  /* Prepare the arguments array */
  emacs_value args[] = { Qsym, Sfun };

  /* Make the call (2 == nb of arguments) */
  env->funcall (env, Qfset, 2, args);
}

/* Provide FEATURE to Emacs.  */
static void
provide (emacs_env *env, const char *feature)
{
  /* call 'provide' with FEATURE converted to a symbol */

  emacs_value Qfeat = env->intern (env, feature);
  emacs_value Qprovide = env->intern (env, "provide");
  emacs_value args[] = { Qfeat };

  env->funcall (env, Qprovide, 1, args);
}

#ifdef __cplusplus
extern "C" {
#endif
int plugin_is_GPL_compatible;

int emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env * env = ert->get_environment(ert);
  emacs_value search_function = env->make_function(env,
                                                   1,
                                                   1,
                                                   execute_search,
                                                   "Searches for the first artist name in ARGS in musicbrainz.org.",
                                                   nullptr);
  bind_function(env, "musicbrainz-search-artist", search_function);
  provide(env, "musicbrainz");

  return 0;
}
#ifdef __cplusplus
}
#endif