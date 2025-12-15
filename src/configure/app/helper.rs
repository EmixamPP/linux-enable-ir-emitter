use std::str::FromStr;

/// Adds to a numeric a digit character if specified. If not, or removes the last digit.
///
/// If all the digits are removed, the numeric is set to its default value.
pub fn add_or_remove_char_from_numeric<T>(numeric: &mut T, ch: Option<char>)
where
    T: ToString + FromStr + Default,
{
    let mut s = numeric.to_string();
    if let Some(c) = ch {
        if !c.is_numeric() {
            return;
        }

        s.push(c);
    } else {
        s.pop();
    }

    if let Ok(new_numeric) = s.parse::<T>() {
        *numeric = new_numeric;
    } else if ch.is_none() {
        // all the characters were removed
        *numeric = T::default();
    }
}

/// Adds to an optional numeric a digit character if specified. If not, removes the last digit.
///
/// If all the digits are removed, the numeric is set to None.
pub fn add_or_remove_char_from_opt_numeric<T>(numeric: &mut Option<T>, ch: Option<char>)
where
    T: ToString + FromStr + Copy,
{
    let mut s = (*numeric).map_or(String::new(), |n| n.to_string());
    if let Some(c) = ch {
        if !c.is_numeric() {
            return;
        }
        s.push(c);
    } else {
        s.pop();
    }

    if let Ok(new_numeric) = s.parse::<T>() {
        *numeric = Some(new_numeric);
    } else if ch.is_none() {
        // all the characters were removed
        *numeric = None;
    }
}

/// Adds to a path a character if specified.
/// If not, removes the last character only if the path length is strictly greater than `nprotect`.
pub fn add_or_remove_char_from_path(
    path: &mut std::path::PathBuf,
    ch: Option<char>,
    nprotect: usize,
) {
    if let Some(path_str) = path.to_str() {
        let mut new_path = path_str.to_string();
        if let Some(c) = ch {
            new_path.push(c);
        } else if new_path.len() - 1 < nprotect {
            return;
        } else {
            new_path.pop();
        }
        *path = std::path::PathBuf::from(new_path);
    }
}

/// Sets a boolean to true or false if the specified character is 't'/'T' or 'f'/'F'.
/// If another or no character is specified, toggles the boolean value.
pub fn add_or_remove_char_from_bool(b: &mut bool, ch: Option<char>) {
    if let Some(c) = ch {
        if c == 't' || c == 'T' {
            *b = true;
        } else if c == 'f' || c == 'F' {
            *b = false;
        }
    } else {
        *b = !*b;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    #[test]
    fn test_add_or_remove_char_from_numeric_add_digit() {
        let mut n = 12u32;
        add_or_remove_char_from_numeric(&mut n, Some('3'));
        assert_eq!(n, 123);
    }

    #[test]
    fn test_add_or_remove_char_from_numeric_remove_digit() {
        let mut n = 123u32;
        add_or_remove_char_from_numeric(&mut n, None);
        assert_eq!(n, 12);
    }

    #[test]
    fn test_add_or_remove_char_from_numeric_remove_all_digits() {
        let mut n = 1u32;
        add_or_remove_char_from_numeric(&mut n, None);
        assert_eq!(n, 0); // Default for u32
        add_or_remove_char_from_numeric(&mut n, None);
        assert_eq!(n, 0); // Default for u32
    }

    #[test]
    fn test_add_or_remove_char_from_numeric_non_digit() {
        let mut n = 12u32;
        add_or_remove_char_from_numeric(&mut n, Some('a'));
        assert_eq!(n, 12);
    }

    #[test]
    fn test_add_or_remove_char_from_opt_numeric_add_digit() {
        let mut n = Some(12u32);
        add_or_remove_char_from_opt_numeric(&mut n, Some('3'));
        assert_eq!(n, Some(123));
    }

    #[test]
    fn test_add_or_remove_char_from_opt_numeric_remove_digit() {
        let mut n = Some(123u32);
        add_or_remove_char_from_opt_numeric(&mut n, None);
        assert_eq!(n, Some(12));
    }

    #[test]
    fn test_add_or_remove_char_from_opt_numeric_remove_all_digits() {
        let mut n = Some(1u32);
        add_or_remove_char_from_opt_numeric(&mut n, None);
        assert_eq!(n, None);
        add_or_remove_char_from_opt_numeric(&mut n, None);
        assert_eq!(n, None);
    }

    #[test]
    fn test_add_or_remove_char_from_opt_numeric_none_add_digit() {
        let mut n: Option<u32> = None;
        add_or_remove_char_from_opt_numeric(&mut n, Some('5'));
        assert_eq!(n, Some(5));
    }

    #[test]
    fn test_add_or_remove_char_from_opt_numeric_non_digit() {
        let mut n = Some(12u32);
        add_or_remove_char_from_opt_numeric(&mut n, Some('x'));
        assert_eq!(n, Some(12));
    }

    #[test]
    fn test_add_or_remove_char_from_path_add_char() {
        let mut path = PathBuf::from("/tmp/test");
        add_or_remove_char_from_path(&mut path, Some('1'), 9);
        assert_eq!(path, PathBuf::from("/tmp/test1"));
    }

    #[test]
    fn test_add_or_remove_char_from_path_remove_char() {
        let mut path = PathBuf::from("/tmp/test1");
        add_or_remove_char_from_path(&mut path, None, 9);
        assert_eq!(path, PathBuf::from("/tmp/test"));
    }

    #[test]
    fn test_add_or_remove_char_from_path_remove_all_chars() {
        let mut path = PathBuf::from("a");
        add_or_remove_char_from_path(&mut path, None, 0);
        assert_eq!(path, PathBuf::from(""));
    }

    #[test]
    fn test_add_or_remove_char_from_path_protected() {
        let mut path = PathBuf::from("a");
        add_or_remove_char_from_path(&mut path, None, 1);
        assert_eq!(path, PathBuf::from("a"));
    }

    #[test]
    fn test_add_or_remove_char_from_bool_set_true() {
        let mut b = false;
        add_or_remove_char_from_bool(&mut b, Some('t'));
        assert!(b);
        add_or_remove_char_from_bool(&mut b, Some('T'));
        assert!(b);
    }

    #[test]
    fn test_add_or_remove_char_from_bool_set_false() {
        let mut b = true;
        add_or_remove_char_from_bool(&mut b, Some('f'));
        assert!(!b);
        add_or_remove_char_from_bool(&mut b, Some('F'));
        assert!(!b);
    }

    #[test]
    fn test_add_or_remove_char_from_bool_toggle() {
        let mut b = true;
        add_or_remove_char_from_bool(&mut b, None);
        assert!(!b);
        add_or_remove_char_from_bool(&mut b, None);
        assert!(b);
    }

    #[test]
    fn test_add_or_remove_char_from_bool_other_char() {
        let mut b = true;
        add_or_remove_char_from_bool(&mut b, Some('x'));
        assert!(b);
        let mut b = false;
        add_or_remove_char_from_bool(&mut b, Some('y'));
        assert!(!b);
    }
}
