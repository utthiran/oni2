type t =
  | Sequence(list(KeyPress.t))
  | AllKeysReleased;

type sequence = list(t);

let parse = (~getKeycode, ~getScancode, str) => {
  let parse = lexbuf =>
    switch (Matcher_parser.main(Matcher_lexer.token, lexbuf)) {
    | exception Matcher_lexer.Error => Error("Error parsing binding: " ++ str)
    | exception (Matcher_lexer.UnrecognizedModifier(m)) =>
      Error("Unrecognized modifier:" ++ m ++ " in: " ++ str)
    | exception Matcher_parser.Error =>
      Error("Error parsing binding: " ++ str)
    | v => Ok(v)
    };

  let flatMap = (f, r) => Result.bind(r, f);

  let finish = r => {
    let f = ((key, mods)) => {
      switch (getKeycode(key), getScancode(key)) {
      | (Some(keycode), Some(scancode)) =>
        Ok(
          KeyPress.{
            modifiers: Matcher_internal.Helpers.internalModsToMods(mods),
            scancode,
            keycode,
          },
        )
      | _ => Error("Unrecognized key: " ++ Key.toString(key))
      };
    };

    switch (r) {
    | Matcher_internal.AllKeysReleased => Ok(AllKeysReleased)
    | Matcher_internal.Sequence(keys) =>
      keys
      |> List.map(f)
      |> Base.Result.all
      |> Result.map(keys => Sequence(keys))
    };
  };

  str
  |> String.lowercase_ascii
  |> Lexing.from_string
  |> parse
  |> flatMap(finish);
};
